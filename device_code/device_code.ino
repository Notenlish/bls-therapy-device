#include <WiFi.h>
#include <WiFiMulti.h>

#include <secrets.h>

const char *host = "192.168.1.111";  // ip or dns
const uint16_t port = 8290;

WiFiMulti WiFiMulti;
WiFiClient client;  // or use NetworkClient?
const int LED_PIN = 8;
bool led_active = false;
bool led_state = false;

// blink
const uint32_t BLINK_MS = 200;
uint32_t lastBlink = 0;

// reconnect
uint32_t lastConnectAttempt = 0;
const uint32_t RECONNECT_MS = 2000;

// Line buffer for commands
String rxLine;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAAAA");
  delay(10);

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.println();
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  // not being able to get local ip.

  delay(500);
}



// LED stuff
void handleLedTask() {
  // Serial.print("led_state:");
  // Serial.print(led_state);
  // Serial.println("PLEASE WORK!!");
  if (led_active) {
    uint32_t now = millis();
    if (now - lastBlink >= BLINK_MS) {
      lastBlink = now;
      led_state = !led_state;
    }

    digitalWrite(LED_PIN, led_state ? LOW : HIGH);
    // in esp32c3 the led pin is reverted for some reason.
    
    return;
  } else {
    digitalWrite(LED_PIN, HIGH);
    // in esp32c3 the led pin is reverted for some reason.
  }
}


// process commands
void processCommand(const String& line) {
  if (line.length() < 4) return;
  String cmd = line.substring(0, 4);

  if (cmd == "STRT") {
    led_active = true;
    digitalWrite(LED_PIN, LOW);  // esp32c3 does the reverse. LOW --> led is active
    client.println("OK STRT");
    Serial.println("STARTING!!");
  } else if (cmd == "STOP") {
    led_active = false;
    digitalWrite(LED_PIN, HIGH);  // esp32c3 does the reverse. LOW --> led is active
    Serial.println("STOPPING ALRIGHT!!");
    client.println("OK STOP");
  } else {
    client.println("ERR UNKNOWN");
  }
}


// network stuff
void handleNetworkTask() {
  // If disconnected, try to reconnect occasionally
  if (!client.connected()) {
    uint32_t now = millis();
    if (now - lastConnectAttempt >= RECONNECT_MS) {
      lastConnectAttempt = now;
      Serial.printf("Connecting to %s:%u...\n", host, port);

      if (client.connect(host, port)) {
        Serial.println("Connected!");
        client.println("HELLO from ESP32-C3");
        rxLine = "";
      } else {
        Serial.println("Connect failed.");
      }
    }
    return;
  }

  // Non-blocking read, consume whatever is available
  while (client.available()) {
    char c = (char)client.read();

    // Build lines ending with '\n' (server should send newline)
    if (c == '\r') continue;
    if (c == '\n') {
      // Serial.print("RX: ");
      // Serial.println(rxLine);
      processCommand(rxLine);
      rxLine = "";
    } else {
      // keep buffer reasonable
      if (rxLine.length() < 128) rxLine += c;
      else rxLine = ""; // reset if something weird happens
    }
  }
}




void loop() {
  handleNetworkTask();
  handleLedTask();

}
