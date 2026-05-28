#pragma once

#include <Arduino.h>

#include "interfaces/AIInference.h"
#include "interfaces/UARTBridge.h"
#include "interfaces/WiFiBridge.h"

class DetectionPipeline {
public:
    void begin(
        AIInference *ai,
        UARTBridge *uart,
        WiFiBridge *wifi
    );

    void update();

private:
    AIInference *_ai = nullptr;
    UARTBridge *_uart = nullptr;
    WiFiBridge *_wifi = nullptr;

    uint32_t _lastHeartbeatMs = 0;
};