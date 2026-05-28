#pragma once

#include <Arduino.h>
#include "interfaces/AIInference.h"

class WiFiBridge {
public:
    void begin();
    void update();
    bool isConnected();

    void publishDetection(const DetectionResult &result);
    void publishHeartbeat();

private:
    uint32_t _lastReconnectAttemptMs = 0;
    void connectWiFi();
};