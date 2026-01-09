#ifndef DEVICE_ID
  #define DEVICE_ID 0
#endif

#ifndef DEVMODE
  #define DEVMODE true
#endif

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

