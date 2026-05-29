#pragma once

#include <Arduino.h>
#include <Wire.h>

// =============================================================================
// DetectionResult — guard-protected so it can safely coexist with AIInference.h
// =============================================================================
#ifndef DETECTION_RESULT_DEFINED
#define DETECTION_RESULT_DEFINED
struct DetectionResult {
    bool    detected    = false;
    String  label       = "";
    float   confidence  = 0.0f;
    int     x = 0, y = 0;
    int     w = 0, h = 0;
};
#endif

// =============================================================================
// AIPacket — 27-byte packed struct sent from XIAO slave to ESP32 master
// =============================================================================
#pragma pack(push, 1)
struct AIPacket {
    uint8_t  detected;
    uint8_t  confidence;
    char     label[12];
    int16_t  x, y, w, h;
    uint32_t uptime;
    uint8_t  checksum;
};
#pragma pack(pop)

static_assert(sizeof(AIPacket) == 27, "AIPacket size mismatch");

// =============================================================================
// I2CBridge — XIAO ESP32-S3 I2C slave on Wire1 (GPIO3 SDA / GPIO4 SCL)
// ESP32 master polls this slave at address AI_I2C_SLAVE_ADDR every 200 ms.
// =============================================================================
class I2CBridge {
public:
    void begin();
    void updateDetection(const DetectionResult &result);
    void clearDetection();
    void sendHeartbeat();

private:
    static volatile AIPacket _packet;
    static uint8_t           _calcChecksum(const AIPacket &p);
    static void              _onRequest();
};