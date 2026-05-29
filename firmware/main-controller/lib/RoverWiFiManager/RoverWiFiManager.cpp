#include "RoverWiFiManager.h"

#include <WiFi.h>
#include <esp_wifi.h>

#include "config/AppConfig.h"
#include "config/NetworkConfig.h"
#include "config/Secrets.h"

// =============================================================================
// WiFi + Classic Bluetooth coexistence on ESP32
//
// RULE: esp_wifi_set_ps(WIFI_PS_MIN_MODEM) is MANDATORY whenever Classic BT
// is active. The ESP-IDF coexistence scheduler requires modem sleep to
// time-share the single 2.4 GHz radio. Calling WIFI_PS_NONE with BT active
// causes an immediate abort() — it is a hard firmware restriction, not a bug.
//
// Offline resilience:
//   reconnect() is fully bounded — it always returns within
//   WIFI_MAX_ATTEMPTS × (WIFI_CONNECT_TIMEOUT_MS + WIFI_RETRY_DELAY_MS).
//   With the defaults that is 3 × (6 s + 1.5 s) = ~22 s worst case.
//   Only TaskNetworkService is affected during that window.
//   TaskBluetoothControl, TaskSensorRead, and TaskGPSRead run uninterrupted
//   on their own RTOS tasks throughout any WiFi outage.
//
// Auto-reconnect:
//   update() is called from TaskNetworkService every 50 ms.
//   When the link is down it checks WIFI_RECONNECT_INTERVAL_MS (20 s).
//   As soon as a reconnect succeeds OTAUpdate and ThingSpeak resume
//   automatically — no reboot needed.
// =============================================================================

void RoverWiFiManager::begin() {
    Serial.println("[WIFI] Initializing Wi-Fi");

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
// reconnect() — always returns; never blocks forever.
//
// AUTH_EXPIRE (reason 2) is expected on the first attempt when BT is active
// because a BT radio burst can interrupt the 4-way handshake mid-flight.
// Waiting WIFI_RETRY_DELAY_MS between attempts lets BT traffic die down so
// the next attempt usually finds a quiet gap to complete authentication.
// -----------------------------------------------------------------------------
void RoverWiFiManager::reconnect() {
    for (uint8_t attempt = 1; attempt <= WIFI_MAX_ATTEMPTS; attempt++) {

        Serial.printf("[WIFI] Connecting to \"%s\" (attempt %u/%u)\n",
                      WIFI_SSID, attempt, WIFI_MAX_ATTEMPTS);

        WiFi.disconnect(true);
        delay(300);                           // let BT traffic settle

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(true);
        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);   // must stay enabled

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

    // All attempts exhausted — system continues offline.
    // TaskBluetoothControl, TaskSensorRead, TaskGPSRead are unaffected.
    // update() will try again after WIFI_RECONNECT_INTERVAL_MS.
    Serial.println("[WIFI] Offline — all tasks continue. Will retry in 20 s");
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