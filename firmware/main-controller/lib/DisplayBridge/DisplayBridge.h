#pragma once

#include <Arduino.h>
#include "TelemetryData.h"

class DisplayBridge {
public:
    void begin();
    void update(const TelemetryData &data);

private:
    uint32_t _lastUpdateMs = 0;
};
