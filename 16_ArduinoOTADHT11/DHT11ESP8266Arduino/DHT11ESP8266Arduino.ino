#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoOTA.h>

#define Relay1 D8
#define BUTTON_PIN D6  // ESP32 pin GPIO22 connected to button's pin
#define DHT_PIN D7     // ESP32 pin GPIO5 connected to DHT11 sensor's pin
#define DHT_TYPE DHT11 // DHT11 sensor type

// const char *ssid = "Famous Meeting Room";
// const char *password = "Bewithus";
const char *ssid = "ARDYAN";
const char *password = "123456789";
const char *mqtt_server = "test.mosquitto.org"; // Alamat IP lokal dari Raspberry Pi

#define sub1 "device1/andrir/relay1"
#define TOPIC_TEMPERATURE "esp32/andrir/temperature"
#define TOPIC_HUMIDITY "esp32/andrir/humidity"

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

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
}

void loop()
{
    ArduinoOTA.handle();

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
    float temperature = 38.1;
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