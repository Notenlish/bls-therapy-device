
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>

#include <secrets.h>
#include <device_types.h>
#include <device_utils.h>
#include <device_setup.h>

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


// TODO: Go check esp32c3 mini pinouts to see what pins should be used for status led.

Preferences prefs;

TherapyDevice::DeviceMode device_mode = TherapyDevice::DeviceMode::Setup;
TherapyDevice::MotorState motor_state = TherapyDevice::MotorState::Active;


WiFiMulti WiFiMulti;  // TODO: I am pretty sure I dont need both wifi and wifimulti
WiFiClient client;    // or use NetworkClient?
const int LED_PIN = 4;


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

void setup() {
  String ssid_list[MAX_SSIDS];
  reserveSsidList(ssid_list);

  // TODO: I forgot to do the actual thing, which is giving the webpage the ability to send over the wifi credentials. im dumb .d
  prefs.begin("wifi", true);
  String ssid = prefs.getString("ssid", "");
  String password = prefs.getString("password", "");
  prefs.end();

  if (ssid.length() == 0) {
    // open softap mode and wait for connection.
    device_mode = TherapyDevice::DeviceMode::Setup;
    ssid = generateSSID(device_info);
    bool softAP_active = WiFi.softAP(ssid);
    if (softAP_active) {
      Serial.println("AP created successfully!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.softAPIP());
      
      int numSSID = WiFi.scanNetworks();
      if (numSSID == -1) {
        Serial.println("Couldn't get a wifi connection");
        return;  // TODO: what would I even do here?
      }

      
      for(int net=0; net<numSSID; net++) {
        String netSSID = WiFi.SSID(net);
        ssid_list[net] = netSSID;
      }

      WiFiServer server(80);
      server.begin();
      return;
    }else {
      Serial.println("Could not create softAP :(");
    }
  } else {
    connected = attemptWifiConnection();
    if (connected) {
      enterPairMode();
    } else {
      enterSetupMode();
    }
  }


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

void loop() {
  if (device_mode == TherapyDevice::DeviceMode::Setup) {
    handleSoftAP(ssid_list);
  }
  handleNetworkTask();
  handleMotorStuff();
}

// ignore pwm if debugging, just set HIGH or LOW depending on state.
void activateMotor(bool state, int pwm, bool left_side, bool right_side) {
  if (IS_MOTOR && state) {
    if (left_side && IS_LEFT_MOTOR) {
      analogWrite(LED_PIN, pwm);
    }
    if (right_side && IS_RIGHT_MOTOR) {
      analogWrite(LED_PIN, pwm);
    }
  } else if (IS_MOTOR && !state) {
    analogWrite(LED_PIN, 0);
  }

  if (!IS_MOTOR) {
    // for debugging I am using the builtin led pin.
    // in the led its the reverse(LOW for turning it on)
    digitalWrite(LED_PIN, state ? LOW : HIGH);
  }
}

void checkSurpassedArea() {
  if (ball_pos < 0.0f) {
    float diff = -ball_pos;
    ball_pos += diff * 2.0f;
    ball_direction *= -1;
  } else if (ball_pos > (float)ball_path_total_duration) {
    float diff = ball_pos - (float)ball_path_total_duration;
    ball_pos -= (float)diff * 2.0f;
    ball_direction *= -1;
  }
}

bool checkMotorShouldActivate(bool& left_side, bool& right_side) {
  left_side = false;
  right_side = false;
  // ball_pos at this point will always be between 0 and ball_path_total_duration
  if (ball_pos <= wall_hit_duration) {
    // at the left wall hit area
    left_side = true;
    return true;
  }
  if (ball_pos >= (wall_hit_duration + between_areas_duration)) {
    // at the right wall hit area
    right_side = true;
    return true;
  }
  return false;
}

// imagine it like this: There is a ball at the start(emdr bls therapy)
// when the tcp server gives the command, the ball starts moving.
// if ball hits the "area" of the left or right wall, the motor will vibrate based on the variable IS_LEFT_MOTOR and IS_RIGHT_MOTOR
// I need to figure out how to make the motor do tiny "start" and "stop" cycles to have variable vibration rates while also keeping the system from desyncing
void handleMotorStuff() {
  // TODO: fix the led_active stuff
  if (ball_active) {  // server gave the START command and ball is moving.
    uint32_t now = millis();
    uint32_t dt = now - lastBallUpdate;
    lastBallUpdate = now;
    if (dt == 0) return;

    ball_pos += ball_speed * (float)dt * (float)ball_direction;

    /* check if it has surpassed the bouncing area
     * this automatically "bounces" the ball_pos in case it goes beyond what its supposed to go to
     * like if ball_pos goes to negatives, it will move it in the opposite direction to keep it in range 
     */
    checkSurpassedArea();

    bool left_side = false;
    bool right_side = false;

    // check if in bouncing area(check whether it should vibrate)
    motor_should_activate = checkMotorShouldActivate(left_side, right_side);

    Serial.printf("ball_pos is %.2f\n", ball_pos);

    // motor vibration
    int pwm = 0;
    if (motor_should_activate) {
      pwm = (float)255 * motorPower;
    }

    activateMotor(motor_should_activate, pwm, left_side, right_side);

  } else {
    // server gave the STOP command.
    int pwm = 0;
    activateMotor(false, pwm, false, false);
  }
}


// process commands
void processCommand(const String& line) {
  if (line.length() < 4) return;
  String cmd = line.substring(0, 4);
  String rest = line.substring(4);

  if (cmd == "STRT") {
    ball_active = true;

    // Reset stuff
    lastBallUpdate = millis();
    ball_direction = 1;
    ball_pos = wall_hit_duration + (between_areas_duration / 2);  // center
    motor_should_activate = false;

    client.println("OK STRT");
  } else if (cmd == "STOP") {
    ball_active = false;
    client.println("OK STOP");

  } else if (cmd == "SETP") {
    motorPower = rest.toFloat();
    motorPower = constrain(motorPower, 0.0f, 1.0f);
    client.print("SET POWER:");
    client.println(String(motorPower));
  } else {
    client.println("ERR UNKNOWN");
  }
}


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
      else rxLine = "";  // reset if something weird happens
    }
  }
}
