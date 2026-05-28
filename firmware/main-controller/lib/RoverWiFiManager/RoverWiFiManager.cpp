#include "RoverWiFiManager.h"

#include <WiFi.h>
#include <esp_wifi.h>

#include "config/AppConfig.h"
#include "config/Secrets.h"

#define WIFI_RECONNECT_INTERVAL_MS 20000
#define WIFI_CONNECT_TIMEOUT_MS    8000

void RoverWiFiManager::begin() {
    Serial.println("[WIFI] Initializing Wi-Fi");

    WiFi.mode(WIFI_STA);

    // Required when Wi-Fi and Classic Bluetooth run together on ESP32
    WiFi.setSleep(true);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);

    _lastReconnectAttempt = 0;

    reconnect();
}

void RoverWiFiManager::update() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    const uint32_t now = millis();

    if ((now - _lastReconnectAttempt) >= WIFI_RECONNECT_INTERVAL_MS) {
        _lastReconnectAttempt = now;
        reconnect();
    }
}

void RoverWiFiManager::reconnect() {
    Serial.println("[WIFI] Attempting connection...");

    WiFi.disconnect(false);
    delay(100);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    const uint32_t startAttempt = millis();

    while (WiFi.status() != WL_CONNECTED &&
           (millis() - startAttempt) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WIFI] Connected");

        Serial.print("[WIFI] IP Address: ");
        Serial.println(WiFi.localIP());

        Serial.print("[WIFI] RSSI: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        Serial.println("[WIFI] Connection failed. System will continue offline.");
    }
}

bool RoverWiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress RoverWiFiManager::getIP() {
    return WiFi.localIP();
}

int32_t RoverWiFiManager::getRSSI() {
    if (WiFi.status() != WL_CONNECTED) {
        return 0;
    }

    return WiFi.RSSI();
}