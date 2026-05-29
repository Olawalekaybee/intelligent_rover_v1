#include "I2CBridge.h"
#include "config/AppConfig.h"

// Static storage — shared between task and ISR
volatile AIPacket I2CBridge::_packet = {};

// =============================================================================
void I2CBridge::begin() {
    // Wire1 as I2C slave on dedicated pins (separate from SSCMA's Wire0)
    Wire1.onRequest(_onRequest);
    Wire1.begin(
        (int)AI_I2C_SLAVE_ADDR,
        (int)AI_I2C1_SDA_PIN,
        (int)AI_I2C1_SCL_PIN
    );

    Serial.printf("[I2C-BRIDGE] Slave ready — addr=0x%02X  SDA=GPIO%d  SCL=GPIO%d\n",
                  AI_I2C_SLAVE_ADDR, AI_I2C1_SDA_PIN, AI_I2C1_SCL_PIN);

    clearDetection();
}

// =============================================================================
// updateDetection — called from TaskDetectionPipeline (RTOS task context)
// =============================================================================
void I2CBridge::updateDetection(const DetectionResult &result) {
    AIPacket p = {};
    p.detected    = result.detected ? 1 : 0;
    p.confidence  = (uint8_t)(result.confidence * 100.0f);
    strncpy(p.label, result.label.c_str(), sizeof(p.label) - 1);
    p.label[sizeof(p.label) - 1] = '\0';
    p.x           = (int16_t)result.x;
    p.y           = (int16_t)result.y;
    p.w           = (int16_t)result.w;
    p.h           = (int16_t)result.h;
    p.uptime      = millis() / 1000;
    p.checksum    = _calcChecksum(p);

    // Atomic-ish copy — 26 bytes, ISR only reads between ESP32 poll cycles
    memcpy((void*)&_packet, &p, sizeof(AIPacket));

    Serial.printf("[I2C-BRIDGE] Detection ready: %s  conf=%u\n",
                  p.label, p.confidence);
}

// =============================================================================
void I2CBridge::clearDetection() {
    AIPacket p = {};
    p.detected = 0;
    p.uptime   = millis() / 1000;
    p.checksum = _calcChecksum(p);
    memcpy((void*)&_packet, &p, sizeof(AIPacket));
}

void I2CBridge::sendHeartbeat() {
    // Update uptime in packet so ESP32 can detect staleness
    AIPacket p;
    memcpy(&p, (void*)&_packet, sizeof(AIPacket));
    p.uptime   = millis() / 1000;
    p.checksum = _calcChecksum(p);
    memcpy((void*)&_packet, &p, sizeof(AIPacket));
}

// =============================================================================
// _onRequest — called in ISR when ESP32 master sends requestFrom(0x55, 26)
// Must be fast — just write the prepared buffer
// =============================================================================
void I2CBridge::_onRequest() {
    Wire1.write((const uint8_t*)&_packet, sizeof(AIPacket));
}

// =============================================================================
uint8_t I2CBridge::_calcChecksum(const AIPacket &p) {
    const uint8_t *b = (const uint8_t*)&p;
    uint8_t xorVal   = 0;
    for (size_t i = 0; i < offsetof(AIPacket, checksum); i++) {
        xorVal ^= b[i];
    }
    return xorVal;
}