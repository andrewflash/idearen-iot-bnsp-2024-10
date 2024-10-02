#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoOTA.h>

// Konfigurasi pin untuk HC-SR04
#define TRIG_PIN 12   // Pin trigger HC-SR04
#define ECHO_PIN 13   // Pin echo HC-SR04
#define OLED_RESET -1 // Pin reset OLED (-1 jika tidak menggunakan reset pada OLED)
#define BUZZER_PIN 14 // Pin Buzzer (GPIO14 / D5 di ESP8266 dan ESP32)

// Inisialisasi objek
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
WiFiClient espClient;
PubSubClient client(espClient);

// Konfigurasi WiFi dan MQTT
const char *ssid = "ARDYAN";
const char *password = "123456789";
const char *mqtt_server = "test.mosquitto.org";

// Pilih salah satu dari konfigurasi ini berdasarkan apakah broker membutuhkan autentikasi
const int mqtt_port_anonymous = 1883;     // Port tanpa autentikasi
const int mqtt_port_authenticated = 8883; // Port dengan autentikasi

// Jika menggunakan autentikasi MQTT (username dan password)
const char *mqtt_username = ""; // Kosongkan jika tidak perlu
const char *mqtt_password = ""; // Kosongkan jika tidak perlu

// Topik terpisah untuk suhu dan kelembapan
const char *topic_jarak = "sensor/hcsr04/idearen-02-10/jarak";
const char *topic_command = "command/idearen-02-10/buzzer";

// Interval waktu pengiriman dalam milidetik (5 detik)
const unsigned long interval = 5000;
unsigned long previousMillis = 0; // Waktu terakhir data dikirim

// Fungsi untuk menghitung jarak dari HC-SR04
float readDistance()
{
    // Bersihkan pin trigger
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Kirim sinyal trigger
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Hitung durasi dari echo
    long duration = pulseIn(ECHO_PIN, HIGH);

    // Hitung jarak dalam cm
    float distance = duration * 0.034 / 2;
    return distance;
}

// Fungsi untuk menampilkan jarak di OLED
void displayData(float jarak)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Jarak: ");
    display.print(jarak);
    display.println(" cm");
    display.display();
}

// Callback saat pesan MQTT diterima
void callback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (int i = 0; i < length; i++)
    {
        msg += (char)payload[i];
    }

    // Jika perintah untuk buzzer diterima
    if (String(topic) == topic_command)
    {
        if (msg == "ON")
        {
            digitalWrite(BUZZER_PIN, HIGH); // Nyalakan buzzer
        }
        else if (msg == "OFF")
        {
            digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer
        }
    }
}

// Fungsi untuk menghubungkan ke WiFi
void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Menghubungkan ke ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi terhubung");
    Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan ke MQTT
void reconnect()
{
    while (!client.connected())
    {
        String clientId = "ESP32Client-" + String(random(0xffff), HEX);

        Serial.print("Menghubungkan ke MQTT...");
        if (mqtt_username && mqtt_password && strlen(mqtt_username) > 0 && strlen(mqtt_password) > 0)
        {
            // Gunakan autentikasi username dan password
            if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
            {
                Serial.println("Terhubung dengan autentikasi");
                client.subscribe(topic_command); // Subscribe ke topik perintah buzzer
            }
        }
        else
        {
            // Koneksi tanpa autentikasi
            if (client.connect(clientId.c_str()))
            {
                Serial.println("Terhubung tanpa autentikasi");
                client.subscribe(topic_command); // Subscribe ke topik perintah buzzer
            }
        }

        if (!client.connected())
        {
            Serial.print("Gagal, rc=");
            Serial.print(client.state());
            Serial.println(" coba lagi dalam 5 detik");
            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer di awal

    // Setup OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Loop forever if OLED initialization fails
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Inisialisasi...");
    display.display();

    // Setup WiFi dan MQTT
    setup_wifi();

    // Tentukan port MQTT berdasarkan apakah menggunakan autentikasi atau tidak
    if (mqtt_username && mqtt_password && strlen(mqtt_username) > 0 && strlen(mqtt_password) > 0)
    {
        client.setServer(mqtt_server, mqtt_port_authenticated); // Port untuk autentikasi
    }
    else
    {
        client.setServer(mqtt_server, mqtt_port_anonymous); // Port tanpa autentikasi
    }

    client.setCallback(callback);

    // Setup OTA
    ArduinoOTA.setPassword("admin"); // Menambahkan password untuk OTA

    ArduinoOTA.onStart([]()
                       {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
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
    } });
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

    // Waktu saat ini
    unsigned long currentMillis = millis();

    // Baca data dari sensor HC-SR04
    float jarak = readDistance();

    // Tampilkan data di OLED
    displayData(jarak);

    // Jika sudah 15 detik
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        // Cek apakah data valid
        if (isnan(jarak))
        {
            Serial.println("Gagal membaca dari sensor HC-SR04!");
            return;
        }

        // Kirim data jarak ke topik MQTT
        String payload_jarak = String(jarak);
        client.publish(topic_jarak, payload_jarak.c_str());
    }
}
