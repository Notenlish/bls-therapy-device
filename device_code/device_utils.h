constexpr Color statusLedColor(TherapyDevice::DeviceMode mode) {
  Color c;
  switch (mode) {
      case TherapyDevice::DeviceMode::Charging:
          c.r = 255;
          c.g = 165; // orange
          c.b = 0;
          return c;
      case TherapyDevice::DeviceMode::Pairing:
          c.r = 255;
          c.g = 255; // white
          c.b = 255;
          return c;
      case TherapyDevice::DeviceMode::Setup:
          c.r = 0;
          c.g = 0;  // blue
          c.b = 255;
          return c;
      case TherapyDevice::DeviceMode::Connected:
          c.r = 0;
          c.g = 255;  // green
          c.b = 0;
          return c;
      default:
          c.r = 0;
          c.g = 0;
          c.b = 0;
          return c; // Off
  }
}

void enterSetupMode(Preferences &prefs) {
  prefs.begin("wifi",false);
  prefs.putString("ssid","");
  prefs.putString("password","");
  prefs.end();

  ESP.restart();
}

// TODO: maybe move these over to a "DeviceUtils" Namespace?
bool attemptWifiConnection(WiFiMulti &wifi_multi) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long start = millis();
    const unsigned long WIFI_TIMEOUT = 10000;

    while (WiFi.status() != WL_CONNECTED &&
        millis() - start < WIFI_TIMEOUT) {
        delay(100);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    } else {
        return false;
    }
}
