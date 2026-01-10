#pragma once

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <device_types.h>
#include <device_constants.h>
#include <Preferences.h>

bool parsePostData(HTTPRequest &http_req, WiFiCredentials &wifi_credentials) {
  JSONVar obj = JSON.parse(http_req.body);
  if (JSON.typeof(obj) == "undefined") {
    Serial.println("Parsing input failed!");
    return false;
  }

  Serial.print("JSON.typeof(obj): ");
  Serial.println(JSON.typeof(obj));

  if (!obj.hasOwnProperty("ssid") || !obj.hasOwnProperty("password")) {
    Serial.println("ssid / password missing.");
    return false;
  }

  if (obj.hasOwnProperty("customssid")) {
    obj["ssid"] = (String)obj["customssid"];
  }

  wifi_credentials.ssid = (String)obj["ssid"];
  wifi_credentials.password = (String)obj["password"];
  return true;
}

void sendPage(WiFiClient &client, String (&ssid_list)[MAX_SSIDS]) {
  // F() is used to make these texts stay in flash and not be copied to ram.
  
  // HTTP response headers
  client.print(F(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Connection: close\r\n"));

  // HTML Body
  client.print(F(
    "<!doctype html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>WiFi Setup</title>"
    "</head><body>"
    "<h1>Select WiFi</h1>"
    "<form method='POST' action='/save'>"
    "<label for='ssid'>SSID</label><br>"
    "<select id='ssid' name='ssid'>"));

  // a bunch of <option> tags
  for (int i = 0; i < MAX_SSIDS; i++) {
    if (ssid_list[i].length() == 0) continue;
    String esc = htmlEscape(ssid_list[i]);
    client.print(F("<option value='"));
    client.print(esc);
    client.print(F("'>"));
    client.print(esc);
    client.print(F("</option>"));
  }
  client.print(F(
    "</select><br><br>"
    "<label for='pass'>Password</label><br>"
    "<input id='pass' name='pass' type='password' autocomplete='off'><br><br>"));

  // custom ssid
  client.print(F(
    "<br><label for='ssid_custom'>SSID custom</label><br>"
    "<input placeholder='Enter SSID here if you cant find it in the options.' type='text' />"));

  client.print(F(
    "<<button type='submit'>Save</button>"
    "</form>"
    "</body></html>"));
}

void sendStatus(WiFiClient &client, bool &successful) {
  int content_length;
  if (successful) content_length = strlen("{ \"valid\":\"true\" }");
  else content_length = strlen("{ \"valid\":\"false\" }");

  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json; charset=utf-8\r\n"
    "Content-Length: "
    "Connection: close\r\n");

  if (successful) client.print("{ \"valid\":\"true\" }");
  else client.print("{ \"valid\":\"false\" }");
}

void handleSoftAP(WiFiServer &server, String (&ssid_list)[MAX_SSIDS], HTTPRequest &http_req, Preferences &prefs) {
  WiFiClient client = server.available();
  if (!client) return;

  bool in_headers = true;
  unsigned long st = millis();
  while (client.connected() && millis() - st < 1000) {
    if (!client.available()) continue;

    // first parse the method, path etc.
    // and then the data(must be json)
    // then try to connect, depending on whether it works or not, then just send back a "successful" or "unsuccessful" message.

    String line = client.readStringUntil('\n');

    if (in_headers) {
      if (line.startsWith("GET")) {
        http_req.method = "get";

        // length of 'GET ' is 4.
        line.substring(4, line.indexOf(" ")).trim();
        http_req.uri = line;
      } else if (line.startsWith("POST")) {
        http_req.method = "post";

        // length of 'POST ' is 5.
        line.substring(5, line.indexOf(" ")).trim();
        http_req.uri = line;
      }

      else if (line.startsWith("Content-Type")) {
        line.substring(strlen("Content-Type: ")).trim();
        http_req.content_type = line;
      } else if (line.startsWith("Content-Length")) {
        line.substring(strlen("Content-Length: ")).trim();
        http_req.content_length = line.toInt();
      }

      else if (line == "\r") {  // if just an empty line(\r\n)
        in_headers = false;
        break;
      }
    }
  }

  if (http_req.method == "get" && http_req.uri == "/") {
    sendPage(client, ssid_list);
  }

  if (http_req.method == "post" && http_req.uri == "/wifi" && http_req.content_length > 0) {
    // wait for all of the body.
    unsigned long bt = millis();
    while (client.connected() && client.available() < http_req.content_length && millis() - bt < 2000) {
      delay(1);
    }


    http_req.body.reserve(http_req.content_length);

    while ((int)http_req.body.length() < http_req.content_length && client.connected()) {
      if (client.available()) {
        http_req.body += (char)client.read();
      }
    }

    WiFiCredentials wifi_credentials;
    bool if_valid_credentials = parsePostData(http_req, wifi_credentials);

    sendStatus(client, if_valid_credentials);

    if (if_valid_credentials) {
      prefs.begin("wifi", true);
      prefs.putString("ssid", wifi_credentials.ssid);
      prefs.putString("password", wifi_credentials.password);
      prefs.end();
      ESP.restart();  // restart to enter the pairing mode.
    }
  }

  client.stop();
}