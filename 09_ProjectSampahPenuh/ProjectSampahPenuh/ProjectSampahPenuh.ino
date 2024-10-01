#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

const int trigPin = 5;
const int echoPin = 18;
const int servoPin = 14;
const int Proksi = 34;

long duration;
int distance;
int deteksi_penuh;
unsigned long tstart = 0;

Servo myServo;

const char *ssid = "Fave";
const char *password = "freewifi";
// const char *mqtt_server = "test.mosquitto.org"; // Alamat IP lokal dari Raspberry Pi
const char *mqtt_server = "mqtt3.thingspeak.com";
const int mqtt_port = 1883;
const char *mqtt_client_id = "MQQeMxoCCh85ARAaKC49MSU";
const char *mqtt_user = "MQQeMxoCCh85ARAaKC49MSU";
const char *mqtt_password = "IhjxF8gak3K3RHjRQ+l0HYyf";
const char *mqtt_topic = "channels/2608048/publish";
long mqttChannelID = 2608048;
// const char *mqtt_topic_command = "channels/2607974/subscribe";

bool pesanTerkirimTutup = false;
bool pesanTerkirimSampahPenuhFull = false;
bool pesanTerkirimSampahPenuhKosong = false;

// Timer
unsigned long tPesanTutup = 0;
unsigned long tPesanSampahFull = 0;
unsigned long tPesanSampahKosong = 0;
const unsigned long intervalSend = 15000;

WiFiClient espClient;
PubSubClient client(espClient);

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

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");

        if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
        {
            Serial.println("connected");
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
    pinMode(27, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(Proksi, INPUT);
    myServo.attach(servoPin);
    myServo.write(0);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, mqtt_port);

    // Set timer
    tPesanSampahFull = millis();
    tPesanSampahKosong = millis();
    tPesanTutup = millis();
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
    Ultrasonik();
    Deteksi_Penuh();
}

void Ultrasonik()
{
    // Memicu sinyal ultrasonik
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Membaca durasi sinyal balik
    duration = pulseIn(echoPin, HIGH);

    // Menghitung jarak
    distance = duration * 0.034 / 2;

    Serial.print(" | Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // Jika jarak kurang dari 10 cm, buka servo
    if (distance < 10)
    {
        myServo.write(90);
        // client.publish("/Deteksi/Penuh", String("Tempat Sampah Terbuka").c_str());// Servo bergerak ke 90 derajat (terbuka)

        // Kirim data ke thingspeak
        if (millis() - tPesanTutup > intervalSend)
        {
            if (pesanTerkirimTutup == false)
            {
                String payload = "field1=" + String(distance) + "&status=MQTTPUBLISH";
                if (client.publish(mqtt_topic, payload.c_str()))
                {
                    Serial.print("Published: ");
                    Serial.println(payload);
                    pesanTerkirimTutup = true;
                }
                else
                {
                    Serial.println("Failed to publish data");
                }
            }
            tPesanTutup = millis();
        }
    }
    else
    {
        myServo.write(0); // Servo kembali ke posisi awal (tertutup)
        pesanTerkirimTutup = false;
    }

    delay(500);
}

void Deteksi_Penuh()
{
    deteksi_penuh = digitalRead(Proksi);
    if (deteksi_penuh == LOW)
    {
        Serial.println("Tempat sampah full");
        digitalWrite(27, HIGH);
        delay(1000);
        digitalWrite(27, LOW);
        delay(1000);
        digitalWrite(27, HIGH);
        delay(10);

        // Kirim ke Thingspeak
        if (millis() - tPesanSampahFull > intervalSend)
        {
            if (pesanTerkirimSampahPenuhFull == false)
            {
                String payload = "field2=1&status=MQTTPUBLISH";
                if (client.publish(mqtt_topic, payload.c_str()))
                {
                    Serial.print("Published: ");
                    Serial.println(payload);
                    pesanTerkirimSampahPenuhFull = true;
                }
                else
                {
                    Serial.println("Failed to publish data");
                }
            }
            tPesanSampahFull = millis();
        }

        pesanTerkirimSampahPenuhKosong = false;
    }
    else
    {
        digitalWrite(27, LOW);
        Serial.println("Tempat sampah kosong");

        if (millis() - tPesanSampahKosong > intervalSend)
        {
            if (pesanTerkirimSampahPenuhKosong == false)
            {
                String payload = "field2=0&status=MQTTPUBLISH";
                if (client.publish(mqtt_topic, payload.c_str()))
                {
                    Serial.print("Published: ");
                    Serial.println(payload);
                    pesanTerkirimSampahPenuhKosong = true;
                }
            }
            else
            {
                Serial.println("Failed to publish data");
            }
            tPesanSampahKosong = millis();
        }

        pesanTerkirimSampahPenuhFull = false;
    }
}
