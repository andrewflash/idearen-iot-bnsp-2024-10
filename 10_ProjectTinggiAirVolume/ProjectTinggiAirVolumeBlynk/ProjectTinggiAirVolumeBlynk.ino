// #define BLYNK_TEMPLATE_ID "TMPL6cK8k8d0V"
// #define BLYNK_TEMPLATE_NAME "tinggidanvolume"

#define BLYNK_TEMPLATE_ID "TMPL68Ox8i0qt"
#define BLYNK_TEMPLATE_NAME "Test OTA"
#define BLYNK_AUTH_TOKEN "5Zr1lyV2cmVYEnObhej1Fgil-w1ZCu3s"
#define BLYNK_PRINT Serial
#define BLYNK_FIRMWARE_VERSION "0.1.1"
#include <WiFi.h>
// #include<BlynkSimpleEsp32.h>
#include "BlynkEdgent.h"

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Fave";
char pass[] = "freewifi";

BlynkTimer timer;

#define echoPin 19 // Pin Echo
#define trigPin 18 // Pin Trigger

long duration;
long jarak;

float tinggiWadah = 5.6;     // Tinggi wadah dalam cm
float luasAlaswadah = 23.75; // Panjang wadah dalam cm2
float tinggiAir1;
float volume;

void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(ssid, pass, "blynk.cloud", 80);
  //  Serial.begin(115200);
  // delay(100);
  BlynkEdgent.begin();
}

void loop()
{
  // Mengukur jarak dengan sensor ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  jarak = duration * 0.034 / 2; // konversi jarak sebenarnya (cm)

  BlynkEdgent.run();

  // Menghitung volume wadah air
  tinggiAir1 = tinggiWadah - jarak;
  volume = tinggiAir1 * luasAlaswadah;

  Blynk.run();
  timer.run();
  Blynk.virtualWrite(V2, tinggiAir1);
  Blynk.virtualWrite(V1, volume);
  delay(2000);

  Serial.print("tinggiAir1: ");
  Serial.print(tinggiAir1);
  Serial.println(" cm");
  Serial.print("volume");
  Serial.print(volume);
  Serial.println(" cm2");
}
