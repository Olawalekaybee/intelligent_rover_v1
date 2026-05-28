#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "config/AppConfig.h"
#include "config/Secrets.h"
#include "protocols/DisplayPacket.h"
#include "CYDDisplay.h"

CYDDisplay display;

static portMUX_TYPE packetMux = portMUX_INITIALIZER_UNLOCKED;
static DisplayPacket latestPacket;
static volatile bool hasPacket = false;
static volatile uint32_t lastPacketMs = 0;

void onI2CReceive(int len);
void setupWiFiAndOTA();
void handleOTA();

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println("[CYD] Booting telemetry display");
    display.begin();
    display.drawBootScreen();

    Wire.begin(static_cast<uint8_t>(CYD_I2C_ADDRESS), CYD_I2C_SDA, CYD_I2C_SCL, CYD_I2C_FREQ);
    Wire.onReceive(onI2CReceive);
    Serial.printf("[CYD] I2C slave ready addr=0x%02X SDA=%d SCL=%d\n", CYD_I2C_ADDRESS, CYD_I2C_SDA, CYD_I2C_SCL);

    setupWiFiAndOTA();
}

void loop() {
    handleOTA();

    static uint32_t lastDraw = 0;
    if ((millis() - lastDraw) >= SCREEN_REFRESH_MS) {
        lastDraw = millis();

        DisplayPacket snapshot;
        bool available;
        uint32_t age;

        portENTER_CRITICAL(&packetMux);
        snapshot = latestPacket;
        available = hasPacket;
        age = millis() - lastPacketMs;
        portEXIT_CRITICAL(&packetMux);

        const bool fresh = available && age < PACKET_TIMEOUT_MS && snapshot.magic == DISPLAY_PACKET_MAGIC;
        display.drawTelemetry(snapshot, fresh);
    }

    delay(20);
}

void onI2CReceive(int len) {
    if (len != static_cast<int>(sizeof(DisplayPacket))) {
        while (Wire.available()) Wire.read();
        return;
    }

    DisplayPacket packet;
    uint8_t *dst = reinterpret_cast<uint8_t*>(&packet);
    for (size_t i = 0; i < sizeof(DisplayPacket) && Wire.available(); ++i) {
        dst[i] = Wire.read();
    }

    if (packet.magic != DISPLAY_PACKET_MAGIC) return;

    portENTER_CRITICAL_ISR(&packetMux);
    latestPacket = packet;
    hasPacket = true;
    lastPacketMs = millis();
    portEXIT_CRITICAL_ISR(&packetMux);
}

void setupWiFiAndOTA() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("[CYD-WIFI] Connecting");
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[CYD-WIFI] Not connected; OTA will retry after reboot");
        return;
    }

    Serial.print("[CYD-WIFI] IP: ");
    Serial.println(WiFi.localIP());

    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
    Serial.printf("[CYD-OTA] Ready: %s.local\n", OTA_HOSTNAME);
}

void handleOTA() {
    if (WiFi.status() == WL_CONNECTED) {
        ArduinoOTA.handle();
    }
}
