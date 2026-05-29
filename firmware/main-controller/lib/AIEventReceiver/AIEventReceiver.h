#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "TelemetryData.h"

// =============================================================================
// AIPacket — must match I2CBridge.h on the XIAO side exactly
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
// AIEventReceiver — ESP32 I2C master on Wire1
// Polls XIAO slave at 0x55 every AI_I2C_POLL_MS.
// Updates TelemetryData when a valid detection packet arrives.
// =============================================================================
class AIEventReceiver {
public:
    void begin();
    void update(TelemetryData &telemetry);
    bool isOnline() const { return _online; }

private:
    uint32_t _lastPollMs      = 0;
    uint32_t _lastHeartbeatMs = 0;
    bool     _online          = false;
    bool     _warnedOffline   = false;

    bool    readPacket(AIPacket &pkt);
    uint8_t calcChecksum(const AIPacket &pkt);
};