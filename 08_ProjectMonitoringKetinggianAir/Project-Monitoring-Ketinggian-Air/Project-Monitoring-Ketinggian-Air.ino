#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Ganti dengan SSID dan Password Wi-Fi Anda
// const char *ssid = "Your_WiFi_SSID";
// const char *password = "Your_WiFi_Password";
const char *ssid = "Fave";
const char *password = "freewifi";

// Ganti dengan informasi broker MQTT Anda
const char *mqtt_server = "mqtt3.thingspeak.com";
const int mqtt_port = 1883;
const char *mqtt_client_id = "EB00DzMTDAsRITsfBTs1KjA";
const char *mqtt_user = "EB00DzMTDAsRITsfBTs1KjA";
const char *mqtt_password = "gWmLhMj3NTlQ43+RqngQpAwz";
const char *mqtt_topic = "channels/2607974/publish";
const char *mqtt_topic_command = "channels/2607974/subscribe";

// Pin untuk HC-SR04
const int trigPin = 5;
const int echoPin = 18;

// Pin untuk LED
const int ledGreenPin = 12;
const int ledRedPin = 14;

WiFiClient espClient;
PubSubClient client(espClient);

// Timer
unsigned long tstartDistance = 0;
const unsigned long intervalDistance = 2000;

unsigned long tstartSend = 0;
const unsigned long intervalSend = 15000;

// Membaca data dari sensor HC-SR04
long duration, distance;

void setup()
{
    Serial.begin(115200);

    // Setup pin HC-SR04
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Setup pin LED
    pinMode(ledGreenPin, OUTPUT);
    pinMode(ledRedPin, OUTPUT);

    // Koneksi Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Setup MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(mqttCallback);

    tstartDistance = millis();
    tstartSend = millis();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // Baca sensor setiap 2 detik
    if(millis() - tstartDistance > intervalDistance)
    {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);

      // Distance dalam cm
      distance = duration * 0.034 / 2;

      Serial.print("Ketinggian ke permukaan air: ");
      Serial.println(distance);

      // Menyalakan LED Hijau jika ketinggian air normal
      if (distance > 50)
      {
          digitalWrite(ledGreenPin, HIGH);
          digitalWrite(ledRedPin, LOW);
      }
      else
      {
          digitalWrite(ledGreenPin, LOW);
          digitalWrite(ledRedPin, HIGH);
      }

      tstartDistance = millis();
    }

    //Kirim data ke MQTT setiap 15 detik
    if(millis() - tstartSend > intervalSend)
    {
      String payload = "field1=" + String(distance) + "&field2=" + digitalRead(ledRedPin) + "&field3=" + digitalRead(ledGreenPin) + "&status=MQTTPUBLISH";
      if (client.publish(mqtt_topic, payload.c_str()))
      {
          Serial.print("Published: ");
          Serial.println(payload);
      }
      else
      {
          Serial.println("Failed to publish data");
      }

      tstartSend = millis();
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
        {
            Serial.println("connected");
            client.subscribe(mqtt_topic_command);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // Fungsi callback untuk menangani pesan yang diterima dari broker MQTT
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    char message[length + 1];

    for (int i = 0; i < length; i++)
    {
        message[i] = (char)payload[i];
    }
    message[length] = '\0';
    Serial.println(message);

    // Parse JSON payload
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Led status merah ada di field2, hijau field3
    const char *ledStateRed = doc["field2"];
    const char *ledStateGreen = doc["field3"];

    // Nyala matikan LED Merah
    if (strcmp(ledStateRed, "1") == 0)
    {
        digitalWrite(2, HIGH);
    }
    else if (strcmp(ledStateRed, "0") == 0)
    {
        digitalWrite(2, LOW);
    }

  // Nyala matikan LED Hijau
    if (strcmp(ledStateGreen, "1") == 0)
    {
        digitalWrite(2, HIGH);
    }
    else if (strcmp(ledStateGreen, "0") == 0)
    {
        digitalWrite(2, LOW);
    }
}