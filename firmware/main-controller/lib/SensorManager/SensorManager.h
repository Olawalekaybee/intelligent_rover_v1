#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ScioSense_ENS160.h>
#include "TelemetryData.h"

class SensorManager {
public:
    bool begin();
    void readEnvironment(TelemetryData &data);

private:
    Adafruit_AHTX0 _aht;
    ScioSense_ENS160 _ens160 = ScioSense_ENS160(ENS160_I2CADDR_1); // usually 0x53

    bool _ahtReady = false;
    bool _ensReady = false;

    float readMQ135Voltage(uint16_t rawValue) const;
};