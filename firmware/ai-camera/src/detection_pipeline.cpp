#include "StreamServer.h"
#include "interfaces/DetectionPipeline.h"
#include "config/AppConfig.h"

void DetectionPipeline::begin(AIInference *ai, I2CBridge *i2c, WiFiBridge *wifi) {
    _ai   = ai;
    _i2c  = i2c;
    _wifi = wifi;
    Serial.println("[PIPELINE] Detection pipeline initialized (I2C bridge)");
}

void DetectionPipeline::onDetection(const DetectionResult &result) {
    if (!_i2c || !_wifi) return;
    uint32_t now = millis();
    if ((now - _lastDetectionMs) < DETECTION_COOLDOWN_MS) return;
    _lastDetectionMs = now;

    _i2c->updateDetection(result);
    _wifi->publishDetection(result);

    StreamServer::notifyDetection(result.label.c_str(),
                                   (uint8_t)(result.confidence * 100),
                                   millis() / 1000);
    Serial.printf("[PIPELINE] %s  conf=%.2f  box=[%d,%d,%d,%d]\n",
                  result.label.c_str(), result.confidence,
                  result.x, result.y, result.w, result.h);
}

void DetectionPipeline::tickHeartbeat() {
    if (!_i2c || !_wifi) return;
    uint32_t now = millis();
    if ((now - _lastHeartbeatMs) >= HEARTBEAT_INTERVAL_MS) {
        _lastHeartbeatMs = now;
        _i2c->sendHeartbeat();
        _wifi->publishHeartbeat();
    }
}