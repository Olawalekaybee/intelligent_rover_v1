#include "OTAUpdate.h"

#include <WiFi.h>
#include <ArduinoOTA.h>

#include "config/NetworkConfig.h"
#include "config/Secrets.h"

void OTAUpdate::begin() {
    if (_started) return;

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] Wi-Fi not connected — will retry when link is up");
        return;
    }

    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(OTA_PORT);

    ArduinoOTA.onStart([]() {
        Serial.println();
        Serial.println("[OTA] Update started");
        Serial.println(ArduinoOTA.getCommand() == U_FLASH
                       ? "[OTA] Type: firmware"
                       : "[OTA] Type: filesystem");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println();
        Serial.println("[OTA] Update finished — rebooting");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t lastPercent = 255;
        uint8_t percent = (progress * 100) / total;
        if (percent != lastPercent) {
            lastPercent = percent;
            Serial.printf("[OTA] %u%%\r", percent);
        }
        delay(1);
        yield();
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);
        switch (error) {
            case OTA_AUTH_ERROR:    Serial.println("Authentication failed"); break;
            case OTA_BEGIN_ERROR:   Serial.println("Begin failed");          break;
            case OTA_CONNECT_ERROR: Serial.println("Connect failed");        break;
            case OTA_RECEIVE_ERROR: Serial.println("Receive failed");        break;
            case OTA_END_ERROR:     Serial.println("End failed");            break;
            default:                Serial.println("Unknown error");          break;
        }
    });

    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.begin();

    delay(100);
    _started = true;

    Serial.printf("[OTA] Ready — hostname: %s.local  IP: %s  port: %d\n",
                  OTA_HOSTNAME, WiFi.localIP().toString().c_str(), OTA_PORT);
}

void OTAUpdate::handle() {
    // Auto-start whenever WiFi comes (back) up
    if (!_started) {
        if (WiFi.status() == WL_CONNECTED) begin();
        return;
    }
    ArduinoOTA.handle();
    delay(1);
    yield();
}