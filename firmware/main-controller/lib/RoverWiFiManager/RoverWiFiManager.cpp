#include "RoverWiFiManager.h"

#include <WiFi.h>
#include <esp_wifi.h>

#include "config/AppConfig.h"
#include "config/NetworkConfig.h"
#include "config/Secrets.h"

// =============================================================================
// WiFi + Classic Bluetooth coexistence — ESP32 rules
//
// 1. WIFI_PS_MIN_MODEM is MANDATORY when Classic BT is active.
//    The single 2.4 GHz radio is time-shared; modem sleep is the mechanism
//    the coexistence scheduler uses. WIFI_PS_NONE causes an abort().
//
// 2. WiFi.disconnect(false) — "false" means soft-disconnect only: the radio
//    stays powered and the driver stays initialised. disconnect(true) shuts
//    the radio down and causes "timeout when WiFi un-init" on the next
//    WiFi.begin() call because the driver hasn't finished de-initialising.
//
// 3. Mode, sleep, and power-save settings are applied ONCE in begin().
//    Repeating them in every reconnect attempt adds unnecessary re-init
//    overhead and risks the type=4 timeout error.
//
// 4. AUTH_EXPIRE (reason 2) happens because a BT burst can interrupt the
//    4-way WPA handshake. A 500 ms pause before auth lets current BT traffic
//    drain. Retrying up to WIFI_MAX_ATTEMPTS handles the cases where the
//    first attempt is unlucky and hits a burst.
// =============================================================================

void RoverWiFiManager::begin() {
    Serial.println("[WIFI] Initializing Wi-Fi");

    // Apply all WiFi settings once here — not repeated in reconnect()
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);   // mandatory with Classic BT
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);

    _lastReconnectAttempt = 0;
    reconnect();
}

void RoverWiFiManager::update() {
    if (WiFi.status() == WL_CONNECTED) return;

    const uint32_t now = millis();
    if ((now - _lastReconnectAttempt) >= WIFI_RECONNECT_INTERVAL_MS) {
        _lastReconnectAttempt = now;
        reconnect();
    }
}

// -----------------------------------------------------------------------------
// reconnect() — always returns; never blocks indefinitely.
//
// Worst-case blocking time:
//   WIFI_MAX_ATTEMPTS × (WIFI_CONNECT_TIMEOUT_MS + WIFI_RETRY_DELAY_MS)
//   = 3 × (10 000 + 1 500) = ~34 s
// Only TaskNetworkService is blocked during this window.
// TaskBluetoothControl, TaskSensorRead, and TaskGPSRead run unaffected.
// -----------------------------------------------------------------------------
void RoverWiFiManager::reconnect() {
    for (uint8_t attempt = 1; attempt <= WIFI_MAX_ATTEMPTS; attempt++) {

        Serial.printf("[WIFI] Connecting to \"%s\" (attempt %u/%u)\n",
                      WIFI_SSID, attempt, WIFI_MAX_ATTEMPTS);

        // Soft disconnect only — keeps the WiFi radio and driver running.
        // disconnect(true) would power the radio off and cause
        // "timeout when WiFi un-init" on the next WiFi.begin().
        WiFi.disconnect(false);

        // Pause before auth — lets any in-flight BT traffic complete so
        // the WiFi 4-way handshake has a quiet window to finish.
        delay(500);

        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        const uint32_t t0 = millis();
        while (WiFi.status() != WL_CONNECTED &&
               (millis() - t0) < WIFI_CONNECT_TIMEOUT_MS) {
            delay(250);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[WIFI] Connected — IP: %s  RSSI: %d dBm\n",
                          WiFi.localIP().toString().c_str(), WiFi.RSSI());
            return;   // success
        }

        Serial.printf("[WIFI] Attempt %u failed\n", attempt);

        if (attempt < WIFI_MAX_ATTEMPTS) {
            Serial.printf("[WIFI] Retrying in %d ms...\n", WIFI_RETRY_DELAY_MS);
            delay(WIFI_RETRY_DELAY_MS);
        }
    }

    // All attempts exhausted — continue offline.
    // OTAUpdate::handle() and ThingSpeakClient::canUpload() both check
    // isConnected() and return immediately when offline. No rover functions
    // are affected. update() will attempt reconnect after WIFI_RECONNECT_INTERVAL_MS.
    Serial.println("[WIFI] Offline — rover continues normally. Retry in 20 s");
}

bool RoverWiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress RoverWiFiManager::getIP() {
    return WiFi.localIP();
}

int32_t RoverWiFiManager::getRSSI() {
    return (WiFi.status() == WL_CONNECTED) ? WiFi.RSSI() : 0;
}