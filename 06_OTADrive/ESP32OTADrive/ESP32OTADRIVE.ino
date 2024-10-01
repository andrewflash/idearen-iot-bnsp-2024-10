#include <Arduino.h>
#include <otadrive_esp.h>
#include <WiFi.h> // Menambahkan header file WiFi.h

String FW_VERSION = "1.0.4";

void onUpdateProgress(int progress, int totalt)
{
    static int last = 0;
    int progressPercent = (100 * progress) / totalt;
    Serial.print("*");
    if (last != progressPercent && progressPercent % 10 == 0)
    {
        // print every 10%
        Serial.printf("%d", progressPercent);
    }
    last = progressPercent;
}

void setup()
{
    OTADRIVE.setInfo("TOKEN", "v@" + FW_VERSION);
    OTADRIVE.onUpdateFirmwareProgress(onUpdateProgress);

    Serial.begin(115200);

    WiFi.begin("Indobot Academy", "indobot2015");
    pinMode(LED_BUILTIN, OUTPUT);
}

void sync_task()
{
    // Mekanisme timing sederhana untuk mengurangi koneksi ke server
    // Check setiap 60 detik
    if (!OTADRIVE.timeTick(60))
        return;

    if (WiFi.status() != WL_CONNECTED)
        return;

    // Print versi firmware yang sedang berjalan
    Serial.println("Firmware version: " + FW_VERSION);

    // Lakukan operasi sinkronisasi dan pembaruan di sini
    OTADRIVE.updateFirmware();
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2200);
    sync_task();
}