#pragma once

#include <Arduino.h>
#include "interfaces/AIInference.h"

// =============================================================================
// WiFiBridge — WiFi connection, OTA, and detection event publishing
//
// OTA lifecycle:
//   Callbacks are registered once (_callbacksSet).
//   ArduinoOTA.begin() is called every time WiFi connects or reconnects
//   (_otaStarted resets to false on disconnect) so OTA is always reachable
//   after any link recovery — not just after the initial boot.
// =============================================================================
class WiFiBridge {
public:
    void begin();
    void update();
    bool isConnected();

    void publishDetection(const DetectionResult &result);
    void publishHeartbeat();

private:
    uint32_t _lastReconnectAttemptMs = 0;
    bool     _otaStarted             = false;
    bool     _callbacksSet           = false;

    void connectWiFi();
    void startOTA();
    void registerOTACallbacks();
};