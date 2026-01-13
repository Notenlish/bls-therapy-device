#pragma once

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <Preferences.h>
#include <WiFi.h>
#include <device_constants.h>
#include <device_types.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

  Serial.println("Starting to send page.");
  // HTTP response headers
  client.print(F("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "Connection: close\r\n"
                 "\r\n"));

  // HTML Body
  client.print(
      F("<!doctype html><html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>WiFi Setup</title>"
        "</head><body>"
        "<h1>Select WiFi</h1>"
        "<form method='POST' action='/save'>"
        "<label for='ssid'>SSID</label><br>"
        "<select id='ssid' name='ssid'>"));

  Serial.println("Adding the saved ssid list:");

  Serial.printf("ssid_list base=%p sizeof(String)=%u MAX_SSIDS=%u\n",
                (void *)ssid_list, (unsigned)sizeof(String),
                (unsigned)MAX_SSIDS);
  // a bunch of <option> tags
  for (int i = 0; i < MAX_SSIDS; i++) {
    heap_caps_check_integrity_all(true); // catch corruption early
    Serial.print("sending ssid id: ");
    Serial.println(i);

    if (ssid_list[i].length() == 0)
      continue;

    Serial.println("gonna escape some ssid name");

    String esc = htmlEscape(ssid_list[i]);
    Serial.print("Something something escaped html: ");
    Serial.println(esc);

    client.print(F("<option value='"));
    client.print(esc);
    client.print(F("'>"));
    client.print(esc);
    client.print(F("</option>"));
  }
  client.print(F("</select><br><br>"
                 "<label for='pass'>Password</label><br>"
                 "<input id='pass' name='pass' type='password' "
                 "autocomplete='off'><br><br>"));

  Serial.println("Finished sending the ssids.");
  // custom ssid
  client.print(F("<br><label for='ssid_custom'>SSID custom</label><br>"
                 "<input placeholder='Enter SSID here if you cant find it in "
                 "the options.' type='text' />"));

  client.print(F("<button type='submit'>Save</button>"
                 "</form>"
                 "</body></html>"));
  Serial.println("Finished sending the whole page.");
}

void sendStatus(WiFiClient &client, bool &successful) {
  int content_length;
  if (successful)
    content_length = strlen("{ \"valid\":\"true\" }");
  else
    content_length = strlen("{ \"valid\":\"false\" }");

  client.print("HTTP/1.1 200 OK\r\n"
               "Content-Type: application/json; charset=utf-8\r\n"
               "Content-Length: "
               "Connection: close\r\n");

  if (successful)
    client.print("{ \"valid\":\"true\" }");
  else
    client.print("{ \"valid\":\"false\" }");
}

void handleSoftAP(WiFiServer &server, String (&ssid_list)[MAX_SSIDS],
                  Preferences &prefs) {
  HTTPRequest http_req;
  WiFiClient client = server.available();

  // Serial.print("loop client: ");
  // Serial.println(client);
  if (!client)
    return;
  Serial.println(".");

  bool in_headers = true;
  unsigned long st = millis();
  while (client.connected() && millis() - st < 1000) {
    Serial.println(".");
    if (!client.available()) {
      delay(5);
      continue;
    }

    // first parse the method, path etc.
    // and then the data(must be json)
    // then try to connect, depending on whether it works or not, then just send
    // back a "successful" or "unsuccessful" message.

    String line = client.readStringUntil('\n');
    Serial.print("read line:  ");
    Serial.println(line);

    if (in_headers) {
      if (line.startsWith("GET")) {
        http_req.method = "get";

        http_req.uri =
            line.substring(strlen("GET "), line.indexOf(" ", strlen("GET ")));
        http_req.uri.trim();
        Serial.print("uri: ");
        Serial.println(http_req.uri);
      } else if (line.startsWith("POST")) {
        http_req.method = "post";

        http_req.uri =
            line.substring(strlen("POST "), line.indexOf(" ", strlen("POST ")));
        http_req.uri.trim();
        Serial.print("uri: ");
        Serial.println(http_req.uri);
      }

      else if (line.startsWith("Content-Type")) {
        http_req.content_type = line.substring(strlen("Content-Type: "));
        http_req.content_type.trim();
        Serial.print("content_type: ");
        Serial.println(http_req.content_type);
      } else if (line.startsWith("Content-Length")) {
        line = line.substring(strlen("Content-Length: "));
        line.trim();
        http_req.content_length = line.toInt();
        Serial.print("content_length: ");
        Serial.println(http_req.content_length);
      }

      else if (line == "\r") { // if just an empty line(\r\n)
        Serial.println("Just an empty line, breaking");
        in_headers = false;
        break;
      }
    }
  }

  heap_caps_check_integrity_all(true); // catch corruption early

  if (http_req.method == "get" && http_req.uri == "/") {
    Serial.println("got GET request, sending back page...");

    Serial.printf("stack high water mark: %u words (%u bytes)\n",
                  (unsigned)uxTaskGetStackHighWaterMark(NULL),
                  (unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
    heap_caps_check_integrity_all(true); // catch corruption early
    sendPage(client, ssid_list);
  }

  if (http_req.method == "post" && http_req.uri == "/wifi" &&
      http_req.content_length > 0) {
    Serial.println("got POST request, waiting for data...");

    // wait for all of the body.
    unsigned long bt = millis();
    while (client.connected() && client.available() < http_req.content_length &&
           millis() - bt < 2000) {
      delay(1);
    }

    http_req.body.reserve(http_req.content_length);

    while ((int)http_req.body.length() < http_req.content_length &&
           client.connected()) {
      if (client.available()) {
        http_req.body += (char)client.read();
      }
    }

    WiFiCredentials wifi_credentials;
    bool if_valid_credentials = parsePostData(http_req, wifi_credentials);

    Serial.print("is wifi credentials valid: ");
    Serial.println(if_valid_credentials);

    sendStatus(client, if_valid_credentials);

    if (if_valid_credentials) {
      prefs.begin("wifi", false); // false --> read-write mode
      prefs.putString("ssid", wifi_credentials.ssid);
      prefs.putString("password", wifi_credentials.password);
      prefs.end();
      ESP.restart(); // restart to enter the pairing mode.
    }
  }

  client.stop();

  Serial.println("I hate you esp32");
}