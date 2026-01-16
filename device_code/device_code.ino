#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>

#include <device_types.h>

#include <device_constants.h>
#include <secrets.h>

#include <device_utils.h>
#include <device_setup.h>
#include <device_pair.h>

#include <ESPmDNS.h>


// TODO: Go check esp32c3 mini pinouts to see what pins should be used for status led.
// TODO: the form on setup mode doesnt correctly respond back. It should send back a page that says "successful" or "not successful" on it.
// TODO: add a button on the web ui to erase all wifi data(so that the new wifi details can be setup again).
//   (aka allow the user to remove everything in the NVS)

Preferences prefs;

TherapyDevice::DeviceMode device_mode = TherapyDevice::DeviceMode::Setup;
TherapyDevice::MotorState motor_state = TherapyDevice::MotorState::Active;



// for simulating ball going from left to right to left etc.
int time_passed = 0;
int wall_hit_duration = 50;
int between_areas_duration = 500;  // the empty area between the sides where it bounces
bool ball_active = false;          // TODO: make the switch on and off stuff for variable motor power
bool motor_should_activate = false;
int ball_direction = 1;                                               // 1(right) or -1(left)
float ball_pos = wall_hit_duration + (between_areas_duration / 2.0);  // center of map
int ball_path_total_duration = (2 * wall_hit_duration) + between_areas_duration;
int ball_period = 4000;  // ms
float ball_speed = (2.0 * ball_path_total_duration) / (float)ball_period;

uint32_t lastBallUpdate = 0.0;  // ms

float motorPower = 0.5f;


// reconnect
uint32_t lastConnectAttempt = 0;
const uint32_t RECONNECT_MS = 2000;

// Line buffer for commands
String rxLine;

WiFiServer server(80);
String ssid_list[MAX_SSIDS];
void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(2000);
  Serial.println("PLEASE work");

  reserveSsidList(ssid_list);

  prefs.begin("wifi");  // Readonly mode
  WiFiCredentials saved_credentials(prefs.getString("ssid", ""), prefs.getString("password", ""));
  prefs.end();

  if (saved_credentials.ssid.length() == 0) {
    Serial.println("No saved credentials.");
    // open softap mode and wait for connection.
    device_mode = TherapyDevice::DeviceMode::Setup;
    String softAP_ssid = generateSSID(device_info);
    bool softAP_active = WiFi.softAP(softAP_ssid);
    if (softAP_active) {
      Serial.println("AP created successfully!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.softAPIP());

      int numSSID = WiFi.scanNetworks();
      if (numSSID == -1) {
        Serial.println("Warning: Couldn't get a WiFi connection..");
        // if no wifi exists, who cares. Let the user manually enter an SSID
      }

      for (int net = 0; net < numSSID; net++) {
        String netSSID = WiFi.SSID(net);
        ssid_list[net] = netSSID;
      }

      server.begin();
      Serial.println("Started the server at port 80");
      return;
    } else {
      Serial.println("Could not create softAP :(");
      ESP.restart();
    }
  } else {  // has saved credentials
    Serial.println("Have saved credentials, will attempt to connect.");
    bool connected = attemptWifiConnection(saved_credentials);
    if (connected) {
      Serial.println("Connected to wifi, allowing connection from web app(pairing)");
      enterPairingMode(device_mode, server);
    } else {
      Serial.println("Could not connect to wifi, entering setup mode again.");
      enterSetupMode(prefs);
    }
  }
}

void loop() {
  if (device_mode == TherapyDevice::DeviceMode::Setup) {
    handleSoftAP(server, ssid_list, prefs);
  }
  if (device_mode == TherapyDevice::DeviceMode::Pairing) {
    handlePairing(prefs, server);
  }
  // handleNetworkTask();
  // handleMotorStuff();
}

// // ignore pwm if debugging, just set HIGH or LOW depending on state.
// void activateMotor(bool state, int pwm, bool left_side, bool right_side) {
//   if (IS_MOTOR && state) {
//     if (left_side && IS_LEFT_MOTOR) {
//       analogWrite(LED_PIN, pwm);
//     }
//     if (right_side && IS_RIGHT_MOTOR) {
//       analogWrite(LED_PIN, pwm);
//     }
//   } else if (IS_MOTOR && !state) {
//     analogWrite(LED_PIN, 0);
//   }

//   if (!IS_MOTOR) {
//     // for debugging I am using the builtin led pin.
//     // in the led its the reverse(LOW for turning it on)
//     digitalWrite(LED_PIN, state ? LOW : HIGH);
//   }
// }

// void checkSurpassedArea() {
//   if (ball_pos < 0.0f) {
//     float diff = -ball_pos;
//     ball_pos += diff * 2.0f;
//     ball_direction *= -1;
//   } else if (ball_pos > (float)ball_path_total_duration) {
//     float diff = ball_pos - (float)ball_path_total_duration;
//     ball_pos -= (float)diff * 2.0f;
//     ball_direction *= -1;
//   }
// }

// bool checkMotorShouldActivate(bool& left_side, bool& right_side) {
//   left_side = false;
//   right_side = false;
//   // ball_pos at this point will always be between 0 and ball_path_total_duration
//   if (ball_pos <= wall_hit_duration) {
//     // at the left wall hit area
//     left_side = true;
//     return true;
//   }
//   if (ball_pos >= (wall_hit_duration + between_areas_duration)) {
//     // at the right wall hit area
//     right_side = true;
//     return true;
//   }
//   return false;
// }

// // imagine it like this: There is a ball at the start(emdr bls therapy)
// // when the tcp server gives the command, the ball starts moving.
// // if ball hits the "area" of the left or right wall, the motor will vibrate based on the variable IS_LEFT_MOTOR and IS_RIGHT_MOTOR
// // I need to figure out how to make the motor do tiny "start" and "stop" cycles to have variable vibration rates while also keeping the system from desyncing
// void handleMotorStuff() {
//   // TODO: fix the led_active stuff
//   if (ball_active) {  // server gave the START command and ball is moving.
//     uint32_t now = millis();
//     uint32_t dt = now - lastBallUpdate;
//     lastBallUpdate = now;
//     if (dt == 0) return;

//     ball_pos += ball_speed * (float)dt * (float)ball_direction;

//     /* check if it has surpassed the bouncing area
//      * this automatically "bounces" the ball_pos in case it goes beyond what its supposed to go to
//      * like if ball_pos goes to negatives, it will move it in the opposite direction to keep it in range
//      */
//     checkSurpassedArea();

//     bool left_side = false;
//     bool right_side = false;

//     // check if in bouncing area(check whether it should vibrate)
//     motor_should_activate = checkMotorShouldActivate(left_side, right_side);

//     Serial.printf("ball_pos is %.2f\n", ball_pos);

//     // motor vibration
//     int pwm = 0;
//     if (motor_should_activate) {
//       pwm = (float)255 * motorPower;
//     }

//     activateMotor(motor_should_activate, pwm, left_side, right_side);

//   } else {
//     // server gave the STOP command.
//     int pwm = 0;
//     activateMotor(false, pwm, false, false);
//   }
// }

// // process commands
// void processCommand(const String& line) {
//   if (line.length() < 4) return;
//   String cmd = line.substring(0, 4);
//   String rest = line.substring(4);

//   if (cmd == "STRT") {
//     ball_active = true;

//     // Reset stuff
//     lastBallUpdate = millis();
//     ball_direction = 1;
//     ball_pos = wall_hit_duration + (between_areas_duration / 2);  // center
//     motor_should_activate = false;

//     client.println("OK STRT");
//   } else if (cmd == "STOP") {
//     ball_active = false;
//     client.println("OK STOP");

//   } else if (cmd == "SETP") {
//     motorPower = rest.toFloat();
//     motorPower = constrain(motorPower, 0.0f, 1.0f);
//     client.print("SET POWER:");
//     client.println(String(motorPower));
//   } else {
//     client.println("ERR UNKNOWN");
//   }
// }


// void handleNetworkTask() {
//   // If disconnected, try to reconnect occasionally
//   if (!client.connected()) {
//     uint32_t now = millis();
//     if (now - lastConnectAttempt >= RECONNECT_MS) {
//       lastConnectAttempt = now;
//       Serial.printf("Connecting to %s:%u...\n", host, port);

//       if (client.connect(host, port)) {
//         Serial.println("Connected!");
//         client.println("HELLO from ESP32-C3");
//         rxLine = "";
//       } else {
//         Serial.println("Connect failed.");
//       }
//     }
//     return;
//   }

//   // Non-blocking read, consume whatever is available
//   while (client.available()) {
//     char c = (char)client.read();

//     // Build lines ending with '\n' (server should send newline)
//     if (c == '\r') continue;
//     if (c == '\n') {
//       // Serial.print("RX: ");
//       // Serial.println(rxLine);
//       processCommand(rxLine);
//       rxLine = "";
//     } else {
//       // keep buffer reasonable
//       if (rxLine.length() < 128) rxLine += c;
//       else rxLine = "";  // reset if something weird happens
//     }
//   }
// }
