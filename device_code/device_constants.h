#ifndef DEVICE_ID
  #define DEVICE_ID 0
#endif

#ifndef DEVMODE
  #define DEVMODE true
#endif

#ifndef PAIRING_SECRET
  #if DEVMODE
    #define PAIRING_SECRET "587e2b9145066b54898d8fa28ec6c11f0c288273a865b984e02a0e8196001246"
  #endif
#endif

#define MAX_SSIDS 16
#define LED_PIN 4

#if DEVMODE
constexpr DeviceInfo device_info = {
  .device_id = {
    'D',
    'E',
    'V',
    '0' + (DEVICE_ID / 10),
    '0' + (DEVICE_ID % 10),
    '\0' },
  .pairing_secret = PAIRING_SECRET
};
#else
constexpr device_info = {
  .device_id = {
    '0' + (DEVICE_ID / 10000),
    '0' + (DEVICE_ID / 1000),
    '0' + (DEVICE_ID / 100),
    '0' + (DEVICE_ID / 10),
    '0' + (DEVICE_ID % 10),
    '\0' },
  .pairing_secret = PAIRING_SECRET
};
#endif
