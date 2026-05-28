#pragma once

#include <Arduino.h>

class UARTBridge {
public:
    void begin();

    void sendDetection(
        const String &label,
        float confidence,
        int x,
        int y,
        int w,
        int h
    );

    void sendHeartbeat();

private:
    HardwareSerial aiSerial = Serial1;
};