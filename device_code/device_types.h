struct Color {
  uint8_t r, g, b;
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
  }
};

