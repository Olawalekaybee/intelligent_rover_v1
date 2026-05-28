#include "OTAUpdate.h"

#include <WiFi.h>
#include <ArduinoOTA.h>

#include "config/Secrets.h"

void OTAUpdate::begin() {
    if (_started) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[OTA] Wi-Fi not connected, OTA not started");
        return;
    }

    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.setPort(3232);

    ArduinoOTA.onStart([]() {
        Serial.println();
        Serial.println("[OTA] Update started");

        if (ArduinoOTA.getCommand() == U_FLASH) {
            Serial.println("[OTA] Update type: firmware");
        } else {
            Serial.println("[OTA] Update type: filesystem");
        }
    });

    ArduinoOTA.onEnd([]() {
        Serial.println();
        Serial.println("[OTA] Update finished");
        Serial.println("[OTA] Rebooting...");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t lastPercent = 255;

        uint8_t percent = (progress * 100) / total;

        if (percent != lastPercent) {
            lastPercent = percent;
            Serial.printf("[OTA] Progress: %u%%\n", percent);
        }

        delay(1);
        yield();
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA] Error[%u]: ", error);

        if (error == OTA_AUTH_ERROR) {
            Serial.println("Authentication failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End failed");
        } else {
            Serial.println("Unknown error");
        }
    });

    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.begin();

    delay(100);

    _started = true;

    Serial.println("[OTA] OTA service started");
    Serial.println("[OTA] OTA Ready");

    Serial.print("[OTA] Hostname: ");
    Serial.println(OTA_HOSTNAME);

    Serial.print("[OTA] IP: ");
    Serial.println(WiFi.localIP());
}

void OTAUpdate::handle() {
    if (!_started) {
        if (WiFi.status() == WL_CONNECTED) {
            begin();
        }

        return;
    }

    ArduinoOTA.handle();

    delay(1);
    yield();
}