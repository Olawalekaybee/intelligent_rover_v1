#pragma once

#include <Arduino.h>
#include "config/AppConfig.h"
#include "TelemetryData.h"

class AIEventReceiver {
public:
    void begin();
    void update(TelemetryData &data);

private:
    HardwareSerial _serial = HardwareSerial(2);
    char _buffer[AI_EVENT_BUFFER_SIZE] = {0};
    uint16_t _index = 0;

    void processLine(TelemetryData &data, const char *line);
};
