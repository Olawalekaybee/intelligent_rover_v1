#include "interfaces/DetectionPipeline.h"

#include "config/AppConfig.h"

#define AI_HEARTBEAT_INTERVAL_MS 10000

void DetectionPipeline::begin(
    AIInference *ai,
    UARTBridge *uart,
    WiFiBridge *wifi
) {
    _ai = ai;
    _uart = uart;
    _wifi = wifi;

    Serial.println("[PIPELINE] Detection pipeline initialized");
}

void DetectionPipeline::update() {
    if (_ai == nullptr || _uart == nullptr || _wifi == nullptr) {
        return;
    }

    DetectionResult result = _ai->update();

    if (result.detected) {
        _uart->sendDetection(
            result.label,
            result.confidence,
            result.x,
            result.y,
            result.w,
            result.h
        );

        _wifi->publishDetection(result);
    }

    uint32_t now = millis();

    if ((now - _lastHeartbeatMs) >= AI_HEARTBEAT_INTERVAL_MS) {
        _lastHeartbeatMs = now;

        _uart->sendHeartbeat();
        _wifi->publishHeartbeat();
    }
}