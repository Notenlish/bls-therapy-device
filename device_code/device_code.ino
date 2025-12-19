#include <WiFi.h>
#include <WiFiMulti.h>

#include <secrets.h>

WiFiMulti WiFiMulti;

void setup() {
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
  const uint16_t port = 8290;
  const char *host = "192.168.1.111";  // ip or dns

  Serial.print("Connecting to ");
  Serial.println(host);

  NetworkClient client;  // used to make tcp connections

  if (!client.connect(host, port)) {
    Serial.println("Connection failed.");
    Serial.println("Waiting 5 seconds before retrying...");
    delay(5000);
    return;
  }

  //uncomment this line to send an arbitrary string to the server
  client.print("I, as the client, would like to say hello to you, server.");
  
  //uncomment this line to send a basic document request to the server
  // client.print("GET /index.html HTTP/1.1\n\n");

  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000) {
    maxloops++;
    delay(1);  //delay 1 msec
  }

  if (client.available() > 0) {
    //read back one line from the server
    String line = client.readStringUntil('\r');
    Serial.println(line);
  } else {
    Serial.println("client.available() timed out ");
  }

  Serial.println("Closing connection.");
  client.stop();

  Serial.println("Waiting 5 seconds before restarting...");
  delay(5000);
}
