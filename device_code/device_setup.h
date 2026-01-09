void handleSoftAP(WiFiServer &server, String &ssid_list[MAX_SSIDS]) {
  WiFiClient client = server.available();
  if (!client) return;

  unsigned long st = millis();
  while (client.connected() && millis() - st < 1000) {
    // TODO: actually parse it.
    // first parse the method, path etc.
    // and then the data(must be json)
    // then try to connect, depending on whether it works or not, then just send back a "successful" or "unsuccessful" message.
    if (client.avaible()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }
  }

  // HTTP response headers
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Connection: close\r\n"
  );

  // HTML Body
  client.print(F(
    "<!doctype html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>WiFi Setup</title>"
    "</head><body>"
    "<h1>Select WiFi</h1>"
    "<form method='POST' action='/save'"
    "<label for='ssid'>SSID</label><br>"
    "<select id='ssid' name='ssid'>"
  ));

  // a bunch of <option> tags
  for (int i=0; i<MAX_SSIDS; i++) {
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
    "<input id='pass' name='pass' type='password' autocomplete='off'><br><br>"
  ));

  // custom ssid
  client.print(F(
    "<br><label for='ssid_custom'>SSID custom</label><br>"
    "<input placeholder='Enter SSID here if you cant find it in the options.' type='text' />"
  ));

  client.print(F(
    "<<button type='submit'>Save</button>"
    "</form>"
    "</body></html>"
  ));

  client.stop();
}