#pragma once

#include <Arduino.h>

class OTAUpdate {
public:
    void begin();
    void handle();

private:
    bool _started = false;
};