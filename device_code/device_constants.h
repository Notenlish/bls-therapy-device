#define MAX_SSIDS 128

#if DEVMODE
constexpr DeviceInfo device_info = {
  .deviceId = {
    'D',
    'E',
    'V',
    '0' + (DEVICE_ID / 10),
    '0' + (DEVICE_ID % 10),
    '\0' }
};
#else
constexpr device_info = {
  .deviceId = {
    '0' + (DEVICE_ID / 10000),
    '0' + (DEVICE_ID / 1000),
    '0' + (DEVICE_ID / 100),
    '0' + (DEVICE_ID / 10),
    '0' + (DEVICE_ID % 10),
    '\0' }
};
#endif

#define LED_PIN 4;