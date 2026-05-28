#include "interfaces/AIInference.h"
#include "config/AppConfig.h"

// =============================================================================
// Class label map — matches the model loaded on Grove Vision AI V2
//
// Default Seeed models:
//   Person detection  → target 0 = "person"
//   Face detection    → target 0 = "face"
//   COCO 80-class     → standard COCO labels
//
// Flash your model via SenseCraft AI: https://sensecraft.seeed.cc/ai
// Update this map to match your deployed model.
// =============================================================================
String AIInference::classLabel(int target) {
    switch (target) {
        case 0:  return "person";
        case 1:  return "car";
        case 2:  return "bicycle";
        case 3:  return "animal";
        case 4:  return "fire";
        case 5:  return "smoke";
        default: return "unknown_" + String(target);
    }
}

// =============================================================================
bool AIInference::begin() {
    Wire.begin(GROVE_SDA_PIN, GROVE_SCL_PIN);
    Wire.setClock(GROVE_I2C_FREQ);

    if (!_ai.begin(&Wire)) {
        Serial.println("[AI] Grove Vision AI V2 NOT found on I2C");
        Serial.println("[AI] Check wiring: SDA=GPIO5 SCL=GPIO6");
        _ready = false;
        return false;
    }

    _ready = true;
    Serial.println("[AI] Grove Vision AI V2 initialized");
    Serial.println("[AI] P5V04A Sunny camera active");
    return true;
}

// =============================================================================
// update() — poll Grove Vision AI V2 for latest detection results
//
// Returns the highest-confidence detection above MIN_CONFIDENCE_SCORE.
// Returns empty DetectionResult (detected=false) if nothing found.
// =============================================================================
DetectionResult AIInference::update() {
    DetectionResult result;

    if (!_ready) {
        if ((millis() - _lastRetryMs) > 5000) {
            _lastRetryMs = millis();
            begin();
        }
        return result;
    }

    // invoke(1, false, false) = 1 frame, no filter, non-blocking
    if (_ai.invoke(1, false, false) != 0) {
        return result;  // Not ready yet — will retry next call
    }

    // ── Bounding box detections (object detection model) ──────────────────
    if (!_ai.boxes().empty()) {
        // Find highest confidence detection above threshold
        int bestIdx   = -1;
        int bestScore = MIN_CONFIDENCE_SCORE;

        for (size_t i = 0; i < _ai.boxes().size(); i++) {
            if (_ai.boxes()[i].score > bestScore) {
                bestScore = _ai.boxes()[i].score;
                bestIdx   = (int)i;
            }
        }

        if (bestIdx >= 0) {
            const auto &box = _ai.boxes()[bestIdx];
            result.detected    = true;
            result.label       = classLabel(box.target);
            result.confidence  = box.score / 100.0f;
            result.x           = box.x;
            result.y           = box.y;
            result.w           = box.w;
            result.h           = box.h;

            Serial.printf("[AI] Detected: %s  conf=%.2f  box=[%d,%d,%d,%d]\n",
                          result.label.c_str(), result.confidence,
                          result.x, result.y, result.w, result.h);
        }
        return result;
    }

    // ── Classification results (classification model) ─────────────────────
    if (!_ai.classes().empty()) {
        const auto &cls = _ai.classes()[0];
        if (cls.score >= MIN_CONFIDENCE_SCORE) {
            result.detected   = true;
            result.label      = classLabel(cls.target);
            result.confidence = cls.score / 100.0f;
            result.x          = 0;
            result.y          = 0;
            result.w          = 0;
            result.h          = 0;

            Serial.printf("[AI] Class: %s  conf=%.2f\n",
                          result.label.c_str(), result.confidence);
        }
    }

    return result;
}