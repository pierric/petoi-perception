#include <Arduino.h>
#include <Preferences.h>
#include <M5Atom.h>
#include <WiFi.h>
#include <driver/i2s.h>

#include "mqtt.h"
#include "i2s.h"
#include "ble.h"

#ifndef WIFISSID
#error "WIFISSID (wifi ssid) must be defined."
#define WIFISSID "<ssid>"
#endif

#ifndef WIFIPWD
#error "WIFIPWD (wifi password) must be defined."
#define WIFIPWD "<password>"
#endif

#ifndef MQTT_BROKER_IP
#error "MQTT_BROKER_IP must be defined."
#define MQTT_BROKER_IP "<broker>"
#endif

PetoiBLE petoi;
MqttPetoiClient mqtt(MQTT_BROKER_IP);

void onMessage(void *message, size_t size)
{
    Serial.println((const char *)message);
    petoi.get_command_channel()->writeValue((const char *)message, size);
}

void setup()
{
    M5.begin(true, false, true);
    M5.dis.clear();

    InitI2SSpeakOrMic(MODE_MIC);

    M5.dis.drawpix(0, CRGB(0, 0, 128));
    Serial.println(F("Scanning BLE"));

    petoi.init_scan();
    delay(100);

    Serial.println(F("Connecting Wifi"));
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(WIFI_PS_MIN_MODEM);
    WiFi.begin(WIFISSID, WIFIPWD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println();

    delay(1000);

    mqtt.connect();
    mqtt.subscribe("/petoi/command", onMessage);

    Serial.println(F("WiFi & MQTT Connected."));
    M5.dis.drawpix(0, CRGB(0, 128, 0));
}

void loop()
{
    uint8_t audio_buffer[128];
    size_t n_read_bytes;

    M5.update();
    petoi.init_ble_connection();

    if (M5.Btn.isPressed())
    {
        Serial.println(F("Button pressed."));
    }

    esp_err_t err = i2s_read(I2S_NUM_0, audio_buffer, 128, &n_read_bytes, portMAX_DELAY);
    if (err != ESP_OK)
    {
        Serial.print(F("Failed to record audio frames. error code: "));
        Serial.println(err);
    }
    else
    {
        mqtt.publish("/petoi/hearing", audio_buffer, n_read_bytes);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println(F("Wifi reconnect"));
        WiFi.reconnect();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(100);
        }
    }
    delay(100);
}
