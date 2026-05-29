#include "interfaces/WiFiBridge.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "config/AppConfig.h"
#include "config/Secrets.h"

// =============================================================================
// WiFiBridge — WiFi connection management, OTA, and event publishing
//
// OTA design:
//   registerOTACallbacks() runs exactly once (guarded by _callbacksSet).
//   startOTA() calls ArduinoOTA.begin() and sets _otaStarted = true.
//   update() clears _otaStarted whenever the link drops so startOTA() fires
//   again on the next successful reconnect — making OTA resilient to
//   temporary WiFi outages without duplicating callback registrations.
// =============================================================================

// ── OTA helpers ───────────────────────────────────────────────────────────────

void WiFiBridge::registerOTACallbacks() {
    if (_callbacksSet) return;
    _callbacksSet = true;

    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(3232);

    ArduinoOTA.onStart([]() {
        Serial.println("[OTA] Update started");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA] Finished — rebooting");
    });
    ArduinoOTA.onProgress([](uint32_t done, uint32_t total) {
        Serial.printf("[OTA] %u%%\r", (done * 100) / total);
    });
    ArduinoOTA.onError([](ota_error_t err) {
        Serial.printf("[OTA] Error[%u]\n", err);
    });
    ArduinoOTA.setRebootOnSuccess(true);
}

void WiFiBridge::startOTA() {
    ArduinoOTA.begin();
    _otaStarted = true;
    Serial.printf("[OTA] Ready — %s.local  IP: %s\n",
                  OTA_HOSTNAME, WiFi.localIP().toString().c_str());
}

// ── Public interface ──────────────────────────────────────────────────────────

void WiFiBridge::begin() {
    Serial.println("[WIFI] Starting AI camera WiFi bridge");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Register OTA callbacks once, before the first connect attempt
    registerOTACallbacks();

    connectWiFi();

    // Start OTA immediately if the first connect succeeded
    if (isConnected()) startOTA();
}

void WiFiBridge::update() {
    if (isConnected()) {
        // Start (or restart) OTA whenever we have a live link
        if (!_otaStarted) startOTA();
        ArduinoOTA.handle();
        return;
    }

    // Link is down — reset OTA flag so it restarts after the next reconnect
    if (_otaStarted) {
        _otaStarted = false;
        Serial.println("[WIFI] Link lost — OTA will restart after reconnect");
    }

    // Reconnect watchdog
    uint32_t now = millis();
    if ((now - _lastReconnectAttemptMs) >= WIFI_RECONNECT_INTERVAL_MS) {
        _lastReconnectAttemptMs = now;
        Serial.println("[WIFI] Reconnecting...");
        connectWiFi();
    }
}

bool WiFiBridge::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiBridge::connectWiFi() {
    Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);

    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (!isConnected() && (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    if (isConnected()) {
        Serial.printf("[WIFI] Connected — IP: %s  RSSI: %d dBm\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
    } else {
        Serial.println("[WIFI] Connection failed — will retry");
    }
}

// ── Event publishing ──────────────────────────────────────────────────────────

void WiFiBridge::publishDetection(const DetectionResult &result) {
    if (!result.detected || !isConnected()) return;

    JsonDocument doc;
    doc["device"]     = DEVICE_NAME;
    doc["type"]       = "detection";
    doc["label"]      = result.label;
    doc["confidence"] = result.confidence;
    doc["x"]          = result.x;
    doc["y"]          = result.y;
    doc["w"]          = result.w;
    doc["h"]          = result.h;
    doc["uptime"]     = millis() / 1000;

    String payload;
    serializeJson(doc, payload);
    Serial.print("[WIFI] Detection: ");
    Serial.println(payload);

    // HTTP POST placeholder — wire to ThingSpeak or custom endpoint when ready
    // HTTPClient http;
    // http.begin("http://api.thingspeak.com/update?api_key=...&field1=...");
    // http.GET();
    // http.end();
}

void WiFiBridge::publishHeartbeat() {
    if (!isConnected()) return;

    JsonDocument doc;
    doc["device"] = DEVICE_NAME;
    doc["type"]   = "heartbeat";
    doc["ip"]     = WiFi.localIP().toString();
    doc["rssi"]   = WiFi.RSSI();
    doc["uptime"] = millis() / 1000;

    String payload;
    serializeJson(doc, payload);
    Serial.print("[WIFI] Heartbeat: ");
    Serial.println(payload);
}