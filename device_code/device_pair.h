#include <WiFi.h>
#include <Preferences.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <device_constants.h>
#include <device_types.h>

void enterPairingMode(TherapyDevice::DeviceMode &device_mode, WiFiServer &server) {
  device_mode = TherapyDevice::DeviceMode::Pairing;
  // TODO: status led must be lighting green.
  // TODO: actually write this.

  server.begin();
  Serial.println("Started server at port 80; In pairing mode.");
}

void handlePairing(Preferences &prefs, WiFiServer &server) {
  HTTPRequest http_req;
  WiFiClient client = server.accept();

  if (!client)
    return;
  Serial.println("_");

  bool in_headers = true;
  unsigned long st = millis();
  auto timeout_duration = 1000;
  while (client.connected() && millis() - st < timeout_duration) {
    Serial.println(".");
    if (!client.available()) {
      delay(5);
      continue;
    }

    String line = client.readStringUntil('\n');  // "" --> String, '' --> char
    Serial.print(F("read line:  "));
    Serial.println(line);


    // TODO: move this over to its own function.
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

      else if (line == "\r") {  // if just an empty line(\r\n)
        Serial.println("Just an empty line, breaking");
        in_headers = false;
        break;
      }
    }
  }

  heap_caps_check_integrity_all(true);  // catch corruption early

  if (http_req.method == "get" && http_req.uri == "/") {
    Serial.println("got GET request, sending back page...");
    sendPage(client, ssid_list);
  }

  if (http_req.method == "post" && http_req.uri == "/save" && http_req.content_length > 0) {
    Serial.println("got POST request, waiting for data...");

    // wait for all of the body.
    unsigned long bt = millis();
    while (client.connected() && client.available() < http_req.content_length && millis() - bt < 2000) {
      delay(1);
    }
    http_req.body.reserve(http_req.content_length);

    while ((int)http_req.body.length() < http_req.content_length &&
           client.connected()) {
      if (client.available()) {
        http_req.body += (char)client.read();
      }
    }

    bool if_valid_credentials = parseSetupPostData(http_req, wifi_credentials);
  }
}
