#include <WiFi.h>
#include <WiFiMulti.h>

#include <secrets.h>

const char* host = "192.168.1.111";  // ip or dns
const uint16_t port = 8290;

WiFiMulti WiFiMulti;
WiFiClient client;  // or use NetworkClient?
const int LED_PIN = 8;

bool IS_MOTOR = false;
bool IS_RIGHT_MOTOR = true;
bool IS_LEFT_MOTOR = !IS_RIGHT_MOTOR;


// for simulating ball going from left to right to left etc.
int time_passed = 0;
int wall_hit_duration = 50;
int between_areas_duration = 500;  // the empty area between the sides where it bounces
bool ball_active = false;          // TODO: make the switch on and off stuff for variable motor power
bool motor_should_activate = false;
int ball_direction = 1;                                               // 1(right) or -1(left)
float ball_pos = wall_hit_duration + (between_areas_duration / 2.0);  // center of map
int ball_path_total_duration = (2 * wall_hit_duration) + between_areas_duration;
int ball_period = 2000;  // ms
float ball_speed = (2.0 * ball_path_total_duration) / (float)ball_period;

uint32_t lastBallUpdate = 0.0;  // ms

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
      pwm = 255;
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


void loop() {
  handleNetworkTask();
  handleMotorStuff();
}
