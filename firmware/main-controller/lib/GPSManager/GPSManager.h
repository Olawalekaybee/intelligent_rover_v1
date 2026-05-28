#pragma once

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "TelemetryData.h"

class GPSManager {
public:
    void begin();
    void update();
    void readGPS(TelemetryData &data);

private:
    TinyGPSPlus gps;
    HardwareSerial gpsSerial = HardwareSerial(1);
};