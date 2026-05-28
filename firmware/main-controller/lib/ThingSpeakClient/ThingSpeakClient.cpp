#include "ThingSpeakClient.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "config/AppConfig.h"
#include "config/Secrets.h"

void ThingSpeakClient::begin() {
    Serial.println("[THINGSPEAK] Client initialized");
}

void ThingSpeakClient::update(const TelemetryData &data) {
    if (!canUpload()) return;
    uploadTelemetry(data);
    markUploaded();
}

bool ThingSpeakClient::canUpload() const {
    if (WiFi.status() != WL_CONNECTED) return false;
    return (millis() - _lastUploadMs) >= THINGSPEAK_UPLOAD_INTERVAL_MS;
}

void ThingSpeakClient::markUploaded() {
    _lastUploadMs = millis();
}

void ThingSpeakClient::uploadTelemetry(const TelemetryData &data) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=";
    url += THINGSPEAK_API_KEY;

    // Field mapping:
    // 1 Temp C, 2 Humidity %, 3 eCO2 ppm, 4 TVOC ppb,
    // 5 MQ135 raw, 6 Battery V, 7 Latitude, 8 Longitude
    url += "&field1=" + String(data.temperatureC, 2);
    url += "&field2=" + String(data.humidityPercent, 2);
    url += "&field3=" + String(data.eco2Ppm);
    url += "&field4=" + String(data.tvocPpb);
    url += "&field5=" + String(data.mq135Raw);
    url += "&field6=" + String(data.batteryVoltage, 2);
    url += "&field7=" + String(data.latitude, 6);
    url += "&field8=" + String(data.longitude, 6);

    http.begin(url);
    const int httpCode = http.GET();

    if (httpCode > 0) {
        Serial.printf("[THINGSPEAK] HTTP %d Response: %s\n", httpCode, http.getString().c_str());
    } else {
        Serial.print("[THINGSPEAK] Upload failed: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end();
}
