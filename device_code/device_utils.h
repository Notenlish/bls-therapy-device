#pragma once

String htmlEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      case '\"': out += F("&quot;"); break;
      case '&': out += F("&amp;"); break;
      case '\'': out += F("&#39;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      default: out += c; break;  // do NOT use F() on dynamic stuff, only static strings.
    }
  }
  return out;
}

Color statusLedColor(TherapyDevice::DeviceMode mode) {
  switch (mode) {
    case TherapyDevice::DeviceMode::Charging:
      return { 255, 165, 0 };  // orange
    case TherapyDevice::DeviceMode::Pairing:
      return { 255, 255, 255 };  // white
    case TherapyDevice::DeviceMode::Setup:
      return { 0, 0, 255 };  // blue
    case TherapyDevice::DeviceMode::Connected:
      return { 0, 255, 0 };  // green
    default:
      return { 125, 0, 125 };  // (some purplish color)
  }
}

void enterSetupMode(Preferences &prefs) {
  Serial.println("Entering setup mode.");
  prefs.begin("wifi", false);
  prefs.putString("ssid", "");
  prefs.putString("password", "");
  prefs.end();

  ESP.restart();
}

void enterPairingMode(Preferences &prefs) {
  // TODO: status led must be lighting green.
  // TODO: actually write this.
}

void reserveSsidList(String ssid_list[MAX_SSIDS]) {
  for (int i = 0; i < MAX_SSIDS; i++) {
    ssid_list[i].reserve(32 + 1);
  }
}

// TODO: maybe move these over to a "DeviceUtils" Namespace?
bool attemptWifiConnection(WiFiCredentials saved_credentials) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(saved_credentials.ssid.c_str(), saved_credentials.password.c_str());

  unsigned long start = millis();
  const unsigned long WIFI_TIMEOUT = 7000;

  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  } else {
    return false;
  }
}

String generateSSID(DeviceInfo device_info) {
  String s;
  s = String("SetupTherapyDevice" + String(device_info.device_id));
  return s.c_str();
}
