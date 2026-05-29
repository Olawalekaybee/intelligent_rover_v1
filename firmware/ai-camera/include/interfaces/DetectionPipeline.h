#pragma once

#include <Arduino.h>
#include "I2CBridge.h"          // brings in DetectionResult + AIPacket
#include "interfaces/WiFiBridge.h"

class AIInference;              // forward declaration — full def in AIInference.h

class DetectionPipeline {
public:
    void begin(AIInference *ai, I2CBridge *i2c, WiFiBridge *wifi);
    void onDetection  (const DetectionResult &result);
    void tickHeartbeat();

private:
    AIInference *_ai   = nullptr;
    I2CBridge   *_i2c  = nullptr;
    WiFiBridge  *_wifi = nullptr;

    uint32_t _lastDetectionMs = 0;
    uint32_t _lastHeartbeatMs = 0;
};