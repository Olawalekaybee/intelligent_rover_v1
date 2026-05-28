#pragma once

#include <Arduino.h>

struct TelemetryData {
    // ENS160 + AHT20 environmental module
    float temperatureC = 0.0f;
    float humidityPercent = 0.0f;
    uint16_t eco2Ppm = 0;
    uint16_t tvocPpb = 0;
    uint8_t airQualityIndex = 0;
    bool ahtReady = false;
    bool ens160Ready = false;

    // MQ135 analog gas sensor
    uint16_t mq135Raw = 0;
    float mq135Voltage = 0.0f;

    // GPS telemetry
    double latitude = 0.0;
    double longitude = 0.0;
    double gpsSpeedKmph = 0.0;
    uint8_t gpsSatellites = 0;
    bool gpsValid = false;

    // Power and connectivity
    float batteryVoltage = 0.0f;
    bool wifiConnected = false;
    bool bluetoothConnected = false;
    int8_t wifiRssi = 0;

    // AI vision event from XIAO/Grove Vision AI V2
    bool aiDetected = false;
    char aiLabel[16] = {0};
    float aiConfidence = 0.0f;
    int aiX = 0;
    int aiY = 0;
    int aiW = 0;
    int aiH = 0;

    unsigned long timestampMs = 0;
};
