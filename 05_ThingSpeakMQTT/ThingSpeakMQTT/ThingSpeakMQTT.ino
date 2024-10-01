#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>

// Setting LED
#define LEDPIN 2

// Setting PIN DHT
#define DHT_PIN 16

// Setting WIFI
const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Setting Thingspeak
const char *mqttServer = "mqtt3.thingspeak.com";
const int mqttPort = 1883;
const char *mqttClientId = "NA8aChAoJT0XMScyMTgKIi8";
const char *mqttUsername = "NA8aChAoJT0XMScyMTgKIi8";
const char *mqttPassword = "t8+RLn9AH9478rp09VcVb2SP";
long mqttChannelID = 2590040;

// Timing
const int connectionDelay = 1;
long lastPublishMillis = 0;
const long updateInterval = 15; // dalam detik

// Object
WiFiClient espClient;
PubSubClient client(espClient);
DHTesp dhtSensor;

// Fungsi setup wifi
void connectWifi()
{
    Serial.print("Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi.begin(ssid, password);
        delay(connectionDelay * 1000);
        Serial.print(WiFi.status());
    }
    Serial.println("Connected to Wi-Fi.");
}

// Fungsi MQTT untuk menjalankan fungsi ketika pesan MQTT masuk
void mqttSubscriptionCallback(char *topic, byte *payload, unsigned int length)
{
    // Print detail pesan MQTT yang diterima
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

    // Nyala/matikan LED berdasarkan payload MQTT
    if ((char)payload[0] == '1')
    {
        digitalWrite(LEDPIN, HIGH);
    }
    else
    {
        digitalWrite(LEDPIN, LOW);
    }
    // Parse JSON payload
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error)
    {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
    }

    // Led status ada di field3
    const char *ledState = doc["field3"];

    // Nyala matikan LED
    if (strcmp(ledState, "1") == 0)
    {
        digitalWrite(2, HIGH);
    }
    else if (strcmp(ledState, "0") == 0)
    {
        digitalWrite(2, LOW);
    }
}

// Subscribe ke channel Thingspeak
void mqttSubscribe(long subChannelID)
{
    String myTopic = "channels/" + String(subChannelID) + "/subscribe";
    client.subscribe(myTopic.c_str());
}

// Publish pesan MQTT ke ThingSpeak channel.
void mqttPublish(long pubChannelID, String message)
{
    String topicString = "channels/" + String(pubChannelID) + "/publish";
    client.publish(topicString.c_str(), message.c_str());
}

// Koneksi ke MQTT Server
void mqttConnect()
{
    while (!client.connected())
    {
        if (client.connect(mqttClientId, mqttUsername, mqttPassword))
        {
            Serial.print("MQTT to ");
            Serial.print(mqttServer);
            Serial.print(" at port ");
            Serial.print(mqttPort);
            Serial.println(" successful.");
        }
        else
        {
            Serial.print("MQTT connection failed, rc = ");
            Serial.print(client.state());
            Serial.println(" Will try again in a few seconds");
            delay(connectionDelay * 1000);
        }
    }
}

// Setup Arduino ESP32
void setup()
{
    pinMode(LEDPIN, OUTPUT);
    Serial.begin(9600);

    connectWifi();

    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttSubscriptionCallback);
    client.setBufferSize(2048);

    dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
}

// Loop Arduino ESP32
void loop()
{
    // Connect ke WiFi jika terputus
    if (WiFi.status() != WL_CONNECTED)
    {
        connectWifi();
    }

    // Connect if MQTT client is not connected and resubscribe to channel updates.
    if (!client.connected())
    {
        mqttConnect();
        mqttSubscribe(mqttChannelID);
    }

    // MQTT client loop
    client.loop();

    // Publish MQTT setiap interval yang ditentukan
    if (abs(long(millis()) - lastPublishMillis) > updateInterval * 1000)
    {
        lastPublishMillis = millis();

        TempAndHumidity data = dhtSensor.getTempAndHumidity();
        float suhu = data.temperature;
        float kelembapan = data.humidity;
        int ledPinStatus = digitalRead(LEDPIN);

        // Skip jika data tidak valid
        if (isnan(suhu) || isnan(kelembapan))
        {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        // Buat payload MQTT untuk di publish
        String payload = String("field1=") + String(suhu) +
                         String("&field2=") + String(kelembapan) +
                         String("&field3=") + String(ledPinStatus);

        mqttPublish(mqttChannelID, payload);
        Serial.println("Kirim Payload: " + payload);
    }
}
