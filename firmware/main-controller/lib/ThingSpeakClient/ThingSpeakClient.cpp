#include "ThingSpeakClient.h"

#include <WiFi.h>
#include <HTTPClient.h>

#include "config/AppConfig.h"
#include "config/Secrets.h"

void ThingSpeakClient::begin() {
    Serial.println("[THINGSPEAK] Client initialized");
}

void ThingSpeakClient::update(const TelemetryData &data) {
    if (!canUpload()) {
        return;
    }

    uploadTelemetry(data);
    markUploaded();
}

bool ThingSpeakClient::canUpload() const {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[THINGSPEAK] Skipped: Wi-Fi not connected");
        return false;
    }

    if ((millis() - _lastUploadMs) < THINGSPEAK_UPLOAD_INTERVAL_MS) {
        return false;
    }

    return true;
}

void ThingSpeakClient::markUploaded() {
    _lastUploadMs = millis();
}

void ThingSpeakClient::uploadTelemetry(const TelemetryData &data) {
    HTTPClient http;

    String url = "http://api.thingspeak.com/update?api_key=";
    url += THINGSPEAK_API_KEY;

    url += "&field1=" + String(data.temperatureC, 2);
    url += "&field2=" + String(data.humidityPercent, 2);
    url += "&field3=" + String(data.eco2Ppm);
    url += "&field4=" + String(data.tvocPpb);
    url += "&field5=" + String(data.mq135Raw);
    url += "&field6=" + String(data.latitude, 6);
    url += "&field7=" + String(data.longitude, 6);
    url += "&field8=" + String(data.gpsValid ? data.gpsSatellites : 0);

    Serial.println("[THINGSPEAK] Attempting upload...");
    Serial.print("[THINGSPEAK] Wi-Fi RSSI: ");
    Serial.println(WiFi.RSSI());

    http.setTimeout(5000);

    if (!http.begin(url)) {
        Serial.println("[THINGSPEAK] http.begin() failed");
        http.end();
        return;
    }

    const int httpCode = http.GET();

    if (httpCode > 0) {
        String response = http.getString();

        Serial.printf("[THINGSPEAK] HTTP %d Response: %s\n",
                      httpCode,
                      response.c_str());

        if (httpCode == 200 && response.toInt() > 0) {
            Serial.println("[THINGSPEAK] Upload successful");
        } else if (httpCode == 200 && response.toInt() == 0) {
            Serial.println("[THINGSPEAK] Upload rejected by ThingSpeak");
            Serial.println("[THINGSPEAK] Check update interval, API key, or channel settings");
        }
    } else {
        Serial.print("[THINGSPEAK] Upload failed: ");
        Serial.println(http.errorToString(httpCode));
    }

    http.end();
}