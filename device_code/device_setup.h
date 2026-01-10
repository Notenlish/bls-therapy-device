#pragma once

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <device_types.h>
#include <device_constants.h>

void handleSoftAP(WiFiServer &server, String (&ssid_list)[MAX_SSIDS], HTTPRequest &http_req) {
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
        http_req.uri = line.substring(4, line.indexOf(" ")).trim();
      }
      if (line.startsWith("POST")) {
        http_req.method = "post";

        // length of 'POST ' is 5.
        http_req.uri = line.substring(5, line.indexOf(" ")).trim();
      }

      if (line.startsWith("Content-Type")) {
        http_req.content_type = line.substring(strlen("Content-Type: ")).trim();
      }
      if (line.startsWith("Content-Length")) {
        http_req.content_length = line.substring(strlen("Content-Length: ")).trim().toInt();
      }

      if (line == "\r") {  // if just an empty line(\r\n)
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

    String body;
    body.reserve(contentLength);

    while ((int)body.length() < contentLength && client.connected()) {
      if (client.available()) {
        body += (char)client.read();
      }
    }

    http_req.body = body;
    WiFiCredentials wifi_credentials;
    bool if_valid_credentials = parsePostData(http_req, wifi_credentials);

    sendStatus(client, if_valid_credentials);
  }

  client.stop();
}

bool parsePostData(HttpRequest &http_req, WiFiCredentials &wifi_credentials) {
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
  
  if(obj.hasOwnProperty("customssid")) {
    obj["ssid"] = obj["customssid"];
  }

  wifi_credentials.ssid = obj["ssid"];
  wifi_credentials.password = obj["password"]);
  return true;
}

void sendPage(WiFiClient &client, String (&ssid_list)[MAX_SSIDS]) {
  // HTTP response headers
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Connection: close\r\n");

  // HTML Body
  client.print(F(
    "<!doctype html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>WiFi Setup</title>"
    "</head><body>"
    "<h1>Select WiFi</h1>"
    "<form method='POST' action='/save'"
    "<label for='ssid'>SSID</label><br>"
    "<select id='ssid' name='ssid'>"));

  // a bunch of <option> tags
  for (int i = 0; i < MAX_SSIDS; i++) {
    if (ssid_list[i].length() == 0) continue;
    String esc = htmlEscape(ssid_list[i]);
    client.print(F("<option value='"));
    client.print(F(esc));
    client.print(F("'>"));
    client.print(F(esc));
    Client.print(F("</option>"));
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
  // TODO: implement this.
}