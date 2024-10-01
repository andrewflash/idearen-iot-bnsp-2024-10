#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define Relay1 27
#define BUTTON_PIN 22  // ESP32 pin GPIO22 connected to button's pin
#define DHT_PIN 5      // ESP32 pin GPIO5 connected to DHT11 sensor's pin
#define DHT_TYPE DHT11 // DHT11 sensor type

const char *ssid = "BOH";
const char *password = "officefreewifi";
const char *mqtt_server = "test.mosquitto.org"; // Alamat IP lokal dari Raspberry Pi

#define sub1 "device1/relay1"
#define TOPIC_TEMPERATURE "esp32/temperature"
#define TOPIC_HUMIDITY "esp32/humidity"

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (strcmp(topic, sub1) == 0)
    {
        for (int i = 0; i < length; i++)
        {
            Serial.print((char)payload[i]);
        }
        Serial.println();
        // Switch on the relay if '1' was received as the first character
        if ((char)payload[0] == '1')
        {
            digitalWrite(Relay1, HIGH); // Turn the relay on
        }
        else
        {
            digitalWrite(Relay1, LOW); // Turn the relay off
        }
    }
    else
    {
        Serial.println("unsubscribed topic");
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.publish("outTopic", "hello world");
            client.subscribe(sub1);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup()
{
    pinMode(Relay1, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.begin();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    int buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW)
    {
        Serial.println("Button pressed, toggling relay");
        digitalWrite(Relay1, !digitalRead(Relay1)); // Toggle relay state
        delay(1000);                                // Debounce delay
    }

    // Read temperature and humidity from DHT11 sensor
    // float temperature = dht.readTemperature();
    float temperature = 28.1;
    Serial.print("Temperature : ");
    Serial.println(temperature);
    // float humidity = dht.readHumidity();
    float humidity = 89.1;
    Serial.print("Humidity : ");
    Serial.println(humidity);

    // Publish temperature and humidity to MQTT broker
    if (!isnan(temperature))
    {
        client.publish(TOPIC_TEMPERATURE, String(temperature).c_str());
    }
    if (!isnan(humidity))
    {
        client.publish(TOPIC_HUMIDITY, String(humidity).c_str());
    }

    delay(2000); // Delay between sensor readings
}