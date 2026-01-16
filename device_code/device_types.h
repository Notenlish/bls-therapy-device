#pragma once

struct HTTPRequest {
  String method;
  String uri;
  String body;
  String content_type;
  unsigned int content_length;
};

struct WiFiCredentials {
  String ssid;
  String password;
};

struct Color {
  uint8_t r, g, b;
};

struct DeviceInfo {
  char device_id[6];
  char pairing_secret[64+1];
  char mdns_name[32];
};

namespace Commands {
  enum class CommandType {
    SetSpeed,  // float
    SetMotorPower,  // float
    ScheduleStart,  // unsigned long
    ScheduleEnd,  // unsigned long
    SyncTime,  // unsigned long
  };
};

struct Command {
  Commands::CommandType cmd_type;
  union {
    float f;              // for SetSpeed, SetMotorPower
    unsigned long u32;    // for ScheduleStart, ScheduleEnd, SyncTime
  } value;
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

