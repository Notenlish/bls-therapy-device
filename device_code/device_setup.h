#pragma once

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <Preferences.h>
#include <WiFi.h>
#include <device_constants.h>
#include <device_types.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



void sendPage(WiFiClient &client, String (&ssid_list)[MAX_SSIDS]) {
  // F() is used to make these texts stay in flash and not be copied to ram.
  // HTTP response headers
  client.print(F("HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html; charset=utf-8\r\n"
                 "Connection: close\r\n"
                 "\r\n"));

  // HTML Body
  client.print(F(
      "<!doctype html><html><head>"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
      "<meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\">"
      "<title>WiFi Setup</title>"
      "</head><body>"
      "<h1>Select WiFi</h1>"
      "<form id=\"postForm\" method=\"POST\" action=\"/save\">"
      "<label for=\"ssid\">WiFi Adı (SSID):</label>"
      "<select id=\"ssid\" name=\"ssid\">"));

  // Serial.printf("ssid_list base=%p sizeof(String)=%u MAX_SSIDS=%u\n",
  //               (void *)ssid_list, (unsigned)sizeof(String),
  //               (unsigned)MAX_SSIDS);
  // a bunch of <option> tags
  for (int i = 0; i < MAX_SSIDS; i++) {
    // heap_caps_check_integrity_all(true); // catch corruption early
    if (ssid_list[i].length() == 0)
      continue;

    String esc = htmlEscape(ssid_list[i]);
    // Serial.print("Something something escaped html: ");
    // Serial.println(esc);

    client.print(F("<option value=\""));
    client.print(esc);
    client.print(F("\">"));
    client.print(esc);
    client.print(F("</option>"));
  }
  client.print(F("</select><br>"));

  // custom ssid
  client.print(F("<label for=\"ssid_custom\">WiFi Adı(SSID) Özel:</label>"
                 "<input placeholder=\"Seçeneklerde Ağ Adınızı bulamazsanız "
                 "buradan girin.\" "
                 "type=\"text\" /><br>"));

  client.print(F("<label for=\"pass\">Şifre:</label>"
                 "<div class=\"password-container\">"
                 "<input id=\"password\" name=\"password\" maxlength=\"63\" "
                 "type=\"password\" autocomplete=\"off\" >"
                 "<button id=\"passBtn\" type=\"button\" "
                 "onclick=\"togglePass()\" >Göster</button>"
                 "</div>"
                 "<br>"));

  Serial.println("Finished sending the ssids.");

  client.print(
      F("<br>"
        "<button type=\"submit\">Kaydet</button>"
        "</form>"
        "<style>"
        "body {"
        "padding: 1rem;"
        "background-color: oklch(0.9728 0.01 172.21);"
        "border-radius: 1rem;"
        "}"
        "h1 {"
        "color: oklch(0.4835 0.06 171.15);"
        "width:100%;"
        "text-align:center;"
        "}"
        "form {"
        "padding-block: 1rem;"
        "display: flex;"
        "flex-direction: column;"
        "align-items: center;"
        "}"
        "label {"
        "color: oklch(0.4835 0.06 171.15);"
        "font-size: 0.85rem;"
        "}"
        "select {"
        "border: oklch(0.4496 0.1 145.59) solid 1px;"
        "padding: 0.5rem 0.25rem;"
        "border-radius: 2px;"
        "background-color: white;"
        "}"
        "input {"
        "background-color: white;"
        "border: oklch(0.4496 0.1 145.59) solid 1px;"
        "border-radius: 2px;"
        "padding: 0.5rem 0.25rem;"
        "min-width: max(60vw, 4rem);"
        "}"
        "button {"
        "box-shadow: none;"
        "outline: none;"
        "padding: 0.5rem 1rem;"
        "border-radius: 0.25rem;"
        "font-size: 0.9rem;"
        "background-color: oklch(0.4496 0.1 145.59);"
        "color: white;"
        "}"
        ".password-container {"
        "display: flex;"
        "gap: 0.5rem;"
        "align-items:center;"
        "}"
        "</style>"
        "<script>"
        "const form = document.getElementById(\"postForm\");"
        ""
        "async function sendData() {"
        "const formData = new FormData(form);"
        "const formJsonData = JSON.stringify(Object.fromEntries(formData));"
        "try {"
        "const response = await fetch(\"http://192.168.4.1/save\", {"
        "method:  \"POST\", body: formJsonData });"
        "alert(await response.json());"
        "} catch (e) {"
        "alert(e);"
        "}"
        "}"
        ""
        "form.addEventListener(\"submit\", (event) => { "
        "event.preventDefault(); sendData(); });"
        ""
        "function togglePass(){"
        "let p=document.getElementById('pass');"
        "let b=document.getElementById('passBtn');"
        "if(p.type==='password'){p.type='text';b.textContent='Gizle';}"
        "else{p.type='password';b.textContent='Göster';}"
        "}"
        "</script>"
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


void enterSetupMode(Preferences &prefs) {
  Serial.println("Entering setup mode.");
  prefs.begin("wifi", false);  // false --> Read-Write mode
  prefs.putString("ssid", "");
  prefs.putString("password", "");
  prefs.end();

  ESP.restart();
}


bool parseSetupPostData(HTTPRequest &http_req, WiFiCredentials &wifi_credentials) {
  JSONVar obj = JSON.parse(http_req.body);
  if (JSON.typeof(obj) == "undefined") {
    Serial.println("Parsing input failed!");
    return false;
  }

  Serial.print("JSON.typeof(obj): ");
  Serial.println(JSON.typeof(obj));

  // Serial.print("has ssid: ");
  // Serial.println(obj.hasOwnProperty("ssid"));

  // Serial.print("has password: ");
  // Serial.println(obj.hasOwnProperty("password"));

  if (!obj.hasOwnProperty("ssid") || !obj.hasOwnProperty("password")) {
    Serial.println("ssid / password missing.");
    return false;
  }

  if (obj.hasOwnProperty("ssid_custom")) {
    obj["ssid"] = (String)obj["ssid_custom"];
  }

  wifi_credentials.ssid = (String)obj["ssid"];
  wifi_credentials.password = (String)obj["password"];
  return true;
}


void handleSoftAP(WiFiServer &server, String (&ssid_list)[MAX_SSIDS],
                  Preferences &prefs) {
  HTTPRequest http_req;
  WiFiClient client = server.accept();

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
    // TODO: I forgot to fix the "successful" and "unsuccessful" message thingies.

    String line = client.readStringUntil('\n'); // "" --> String, '' --> char
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

  // heap_caps_check_integrity_all(true); // catch corruption early

  if (http_req.method == "get" && http_req.uri == "/") {
    Serial.println("got GET request, sending back page...");

    // Serial.printf("stack high water mark: %u words (%u bytes)\n",
    //               (unsigned)uxTaskGetStackHighWaterMark(NULL),
    //               (unsigned)uxTaskGetStackHighWaterMark(NULL) * 4);
    // heap_caps_check_integrity_all(true); // catch corruption early
    sendPage(client, ssid_list);
  }

  if (http_req.method == "post" && http_req.uri == "/save" &&
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
    bool if_valid_credentials = parseSetupPostData(http_req, wifi_credentials);
    Serial.print("wifi credentials received:\n");
    Serial.print("ssid: ");
    Serial.print(wifi_credentials.ssid);
    Serial.print("  -  password: ");
    Serial.println(wifi_credentials.password);


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