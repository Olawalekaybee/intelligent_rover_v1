#include "interfaces/WiFiBridge.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "config/AppConfig.h"
#include "config/Secrets.h"

// =============================================================================
// WiFiBridge — manages WiFi connection, OTA, and detection event publishing
// =============================================================================

void WiFiBridge::begin() {
    Serial.println("[WIFI] Starting AI camera Wi-Fi bridge");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    connectWiFi();

    // OTA — only if connected
    if (isConnected()) {
        ArduinoOTA.setHostname(OTA_HOSTNAME);
        ArduinoOTA.setPassword(OTA_PASSWORD);

        ArduinoOTA.onStart([]() {
            Serial.println("[OTA] Update started");
        });
        ArduinoOTA.onEnd([]() {
            Serial.println("\n[OTA] Update finished — rebooting");
        });
        ArduinoOTA.onProgress([](uint32_t progress, uint32_t total) {
            Serial.printf("[OTA] %u%%\r", (progress * 100) / total);
        });
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("[OTA] Error[%u]\n", error);
        });

        ArduinoOTA.begin();
        Serial.printf("[OTA] Ready — hostname: %s.local\n", OTA_HOSTNAME);
    }
}

void WiFiBridge::update() {
    // Handle OTA requests
    if (isConnected()) {
        ArduinoOTA.handle();
        return;
    }

    // WiFi reconnect watchdog
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

    WiFi.disconnect();
    delay(100);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           (millis() - start) < WIFI_CONNECT_TIMEOUT_MS) {
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

    Serial.print("[WIFI] Detection event: ");
    Serial.println(payload);

    // TODO Phase 2 Step 3: HTTP POST to ThingSpeak / cloud endpoint
    // HTTPClient http;
    // http.begin("http://your-endpoint/api/detections");
    // http.addHeader("Content-Type", "application/json");
    // http.POST(payload);
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
    doc["stream"] = REMOTE_STREAM_URL;

    String payload;
    serializeJson(doc, payload);

    Serial.print("[WIFI] Heartbeat: ");
    Serial.println(payload);
}