#include "AIEventReceiver.h"
#include "config/AppConfig.h"
#include "pins/PinMap.h"

void AIEventReceiver::begin() {
    Wire1.begin(PIN_AI_I2C_SDA, PIN_AI_I2C_SCL);
    Wire1.setClock(100000);
    Serial.printf("[AI-I2C] Master ready — SDA=GPIO%d  SCL=GPIO%d  slave=0x%02X\n",
                  PIN_AI_I2C_SDA, PIN_AI_I2C_SCL, AI_I2C_SLAVE_ADDR);
}

void AIEventReceiver::update(TelemetryData &telemetry) {
    uint32_t now = millis();

    // Slow poll when offline (every 2 s), fast poll when online (every 200 ms)
    uint32_t interval = _online ? AI_I2C_POLL_MS : 2000;
    if ((now - _lastPollMs) < interval) return;
    _lastPollMs = now;

    AIPacket pkt;
    if (!readPacket(pkt)) {
        // Only log status change — not every failed poll
        if (_online) {
            _online               = false;
            telemetry.aiNodeOnline = false;
            Serial.println("[AI-I2C] XIAO node offline");
        } else if (!_warnedOffline) {
            _warnedOffline = true;
            Serial.println("[AI-I2C] Waiting for XIAO (0x55) — check wiring + pull-ups");
        }

        // Check long-term timeout
        if ((now - _lastHeartbeatMs) > AI_HEARTBEAT_TIMEOUT_MS) {
            telemetry.aiNodeOnline = false;
        }
        return;
    }

    // Got a valid packet — node is online
    if (!_online) {
        Serial.println("[AI-I2C] XIAO node online");
        _warnedOffline = false;
    }
    _online                = true;
    _lastHeartbeatMs       = now;
    telemetry.aiNodeOnline = true;

    if (pkt.detected) {
        telemetry.aiDetected   = true;
        telemetry.aiConfidence = pkt.confidence / 100.0f;
        strncpy(telemetry.aiLabel, pkt.label, sizeof(telemetry.aiLabel) - 1);
        telemetry.aiLabel[sizeof(telemetry.aiLabel) - 1] = '\0';
        telemetry.aiX = pkt.x;
        telemetry.aiY = pkt.y;
        telemetry.aiW = pkt.w;
        telemetry.aiH = pkt.h;

        Serial.printf("[AI-I2C] %s  conf=%.2f  box=[%d,%d,%d,%d]\n",
                      telemetry.aiLabel, telemetry.aiConfidence,
                      pkt.x, pkt.y, pkt.w, pkt.h);
    } else {
        telemetry.aiDetected = false;
    }
}

bool AIEventReceiver::readPacket(AIPacket &pkt) {
    // Silent ping: beginTransmission/endTransmission does NOT produce
    // a Wire.cpp error log when the device is absent. requestFrom() does —
    // hence we check presence first and only read when device ACKs.
    Wire1.beginTransmission(AI_I2C_SLAVE_ADDR);
    if (Wire1.endTransmission() != 0) return false;  // absent, silent

    Wire1.requestFrom((uint8_t)AI_I2C_SLAVE_ADDR, (uint8_t)AI_I2C_PACKET_SIZE);
    if (Wire1.available() < AI_I2C_PACKET_SIZE) return false;
    Wire1.readBytes((uint8_t*)&pkt, AI_I2C_PACKET_SIZE);
    return pkt.checksum == calcChecksum(pkt);
}

uint8_t AIEventReceiver::calcChecksum(const AIPacket &pkt) {
    const uint8_t *b = (const uint8_t*)&pkt;
    uint8_t x = 0;
    for (size_t i = 0; i < offsetof(AIPacket, checksum); i++) x ^= b[i];
    return x;
}