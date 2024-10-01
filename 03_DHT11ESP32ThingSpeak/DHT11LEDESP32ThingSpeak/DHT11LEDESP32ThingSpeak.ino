#include "ThingSpeak.h"
#include <WiFi.h>

//Replace your wifi credentials here
const char* ssid = "";      //Replace with your Wifi Name
const char* password = "";  // Replace with your wifi Password

//change your channel number here
unsigned long channel = ;  // ID Channel

// angka 3 berarti menandakan field 5 dan 6 yang digunakan untuk membaca state LED
unsigned int led1 = 3;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  ThingSpeak.begin(client);
}

void loop() {

  // untuk membaca data rekam terakhir pada fields
  int led_1 = ThingSpeak.readFloatField(channel, led1);

  if (led_1 == 1) {
    digitalWrite(18, 1);
    Serial.println("LED 1 is On..!");
  } else if (led_1 == 0) {
    digitalWrite(18, 0);
    Serial.println("LED 1 is Off..!");
  }

  Serial.println(led_1);
  delay(5000);
}