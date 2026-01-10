String htmlEscape(const String &s) {
  String out;
  out.reserve(s.length());
  for (unsigned int i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
      // These are the only characters I need to escape, I think.
      case '\"': out += F("&quot;"); break;
      case '&': out += F("&#38;"); break;
      case '\'': out += F("&#39;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
    }
  }
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
  prefs.begin("wifi", false);
  prefs.putString("ssid", "");
  prefs.putString("password", "");
  prefs.end();

  ESP.restart();
}

void reserveSsidList(String ssid_list[MAX_SSIDS]) {
  for (int i = 0; i < MAX_SSIDS; i++) {
    ssid_list[i].reserve(32 + 1);
  }
}

// TODO: maybe move these over to a "DeviceUtils" Namespace?
bool attemptWifiConnection(WiFiMulti &wifi_multi, String ssid, String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

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
  s = "SetupTherapyDevice" + device_info.deviceId;
  return s.c_str();
}
