#ifndef DEVICE_ID
  #define DEVICE_ID 0
#endif

#ifndef DEVMODE
  #define DEVMODE true
#endif

struct HTTPRequest {
  String method;
  String uri;
  String body;
  String content_type;
  unsigned int content_length;
}

struct WiFiCredentials {
  String ssid;
  String password;
}

struct Color {
  uint8_t r, g, b;
};

struct DeviceInfo {
  char deviceId[6];
};

namespace TherapyDevice {
  enum class DeviceMode {
    Charging,
    Setup,
    Pairing,
    Connected,
  };

  enum class MotorState {
    Active,
    InActive,
  };
};

