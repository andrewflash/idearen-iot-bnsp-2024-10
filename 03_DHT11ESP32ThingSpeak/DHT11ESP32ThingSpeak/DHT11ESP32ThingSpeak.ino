#include <DHT.h>
#include <WiFi.h>

String apiKey = "LVBOHBBQ8A2TBB5G";        // sesuaikan dengan Key kalian dari thingspeak.com
const char *ssid = "BOH";     // sesuaikan dengan ssid wifi kalian
const char *password = "officefreewifi"; // sesuaikan dengan wifi password kalian
const char *server = "api.thingspeak.com";

#define DHTPIN 11
#define DHTTYPE DHT11
#define LEDPIN  27

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

bool led_status = false;

void setup()
{
    Serial.begin(115200);
    delay(10);
    dht.begin();
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, LOW);

    WiFi.begin(ssid, password);

    Serial.println();
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
}

void loop()
{

    // float kelembapan = dht.readHumidity();
    // float temperatur = dht.readTemperature();
    float kelembapan = 90.1;
    float temperatur = 30.1;
    if (isnan(kelembapan) || isnan(temperatur))
    {
        Serial.println("Gagal Membaca Sensor DHT");
        return;
    }

    led_status = digitalRead(LEDPIN);

    if (client.connect(server, 80))
    {
        String postStr = apiKey;
        postStr += "&field1=";
        postStr += String(temperatur);
        postStr += "&field2=";
        postStr += String(kelembapan);
        postStr += "&field3=";
        postStr += String(led_status);
        postStr += "\r\n\r\n";

        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);

        Serial.print("Temperatur: ");
        Serial.print(temperatur);
        Serial.println(" *C");
        Serial.print(" Kelembapan: ");
        Serial.print(kelembapan);
        Serial.println(" %");
        Serial.println(" Mengirim Data Thingspeak");
    }
    client.stop();

    Serial.println("Menunggu 1 Detik untuk kirim ke thingspeak.com");
    delay(1000);
}