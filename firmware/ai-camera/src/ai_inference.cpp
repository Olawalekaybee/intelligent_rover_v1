#include "interfaces/AIInference.h"
#include "config/AppConfig.h"
#include <mbedtls/base64.h>

// =============================================================================
// Class label map — update to match the model on your Grove Vision AI V2
// Flash models at: https://sensecraft.seeed.cc/ai
// =============================================================================
String AIInference::classLabel(int target) {
    switch (target) {
        case 0:  return "person";
        case 1:  return "car";
        case 2:  return "bicycle";
        case 3:  return "animal";
        case 4:  return "fire";
        case 5:  return "smoke";
        default: return "object_" + String(target);
    }
}

// =============================================================================
bool AIInference::begin() {
    if (!_wireStarted) {
        Wire.begin(GROVE_SDA_PIN, GROVE_SCL_PIN);
        Wire.setClock(GROVE_I2C_FREQ);
        _wireStarted = true;
    }

    if (!_ai.begin(&Wire)) {
        Serial.println("[AI] Grove Vision AI V2 not found");
        Serial.printf("[AI] Check: SDA=GPIO%d  SCL=GPIO%d\n",
                      GROVE_SDA_PIN, GROVE_SCL_PIN);
        _ready = false;
        return false;
    }

    // Allocate frame buffer from PSRAM
    if (psramFound()) {
        _frameBuffer = (uint8_t*)ps_malloc(MAX_FRAME_BUFFER_SIZE);
        if (_frameBuffer) {
            Serial.printf("[AI] Frame buffer: %d KB in PSRAM\n",
                          MAX_FRAME_BUFFER_SIZE / 1024);
        } else {
            Serial.println("[AI] PSRAM alloc failed — streaming disabled");
        }
    } else {
        Serial.println("[AI] No PSRAM — streaming disabled");
    }

    _ready = true;
    Serial.println("[AI] Grove Vision AI V2 ready");
    Serial.println("[AI] Camera: P5V04A Sunny");
    return true;
}

// =============================================================================
// update() — one inference cycle
//
// captureFrame=false: invoke without image (fast, detection only)
// captureFrame=true : invoke with show=true — gets JPEG with bounding boxes
//                     drawn on it, decodes from base64, stores in _frameBuffer
// =============================================================================
DetectionResult AIInference::update(bool captureFrame) {
    DetectionResult result;

    if (!_ready) {
        // Retry I2C init every 10 s (silent after first warning)
        if ((millis() - _lastRetryMs) > 10000) {
            _lastRetryMs = millis();
            begin();
        }
        return result;
    }

    // invoke(times, filter, show)
    //   show=true  → Grove AI V2 sends back the frame with overlaid boxes
    //   show=false → detection data only (faster, less I2C traffic)
    if (_ai.invoke(1, false, captureFrame) != 0) {
        return result;
    }

    // ── Decode JPEG frame ─────────────────────────────────────────────────
    if (captureFrame && _frameBuffer && !_ai.last_image().isEmpty()) {
        decodeFrame(_ai.last_image());
    }

    // ── Bounding box results (object detection / YOLO models) ─────────────
    if (!_ai.boxes().empty()) {
        int bestIdx   = -1;
        int bestScore = MIN_CONFIDENCE_SCORE;

        for (size_t i = 0; i < _ai.boxes().size(); i++) {
            if (_ai.boxes()[i].score > bestScore) {
                bestScore = _ai.boxes()[i].score;
                bestIdx   = (int)i;
            }
        }

        if (bestIdx >= 0) {
            const auto &box   = _ai.boxes()[bestIdx];
            result.detected   = true;
            result.label      = classLabel(box.target);
            result.confidence = box.score / 100.0f;
            result.x = box.x;  result.y = box.y;
            result.w = box.w;  result.h = box.h;
        }
        return result;
    }

    // ── Classification results (MobileNet models) ─────────────────────────
    if (!_ai.classes().empty()) {
        const auto &cls = _ai.classes()[0];
        if (cls.score >= MIN_CONFIDENCE_SCORE) {
            result.detected   = true;
            result.label      = classLabel(cls.target);
            result.confidence = cls.score / 100.0f;
        }
    }

    return result;
}

// =============================================================================
// decodeFrame — base64 → raw JPEG bytes using mbedTLS (built into ESP-IDF)
// =============================================================================
bool AIInference::decodeFrame(const String &b64) {
    if (b64.isEmpty() || !_frameBuffer) return false;

    int ret = mbedtls_base64_decode(
        _frameBuffer,
        MAX_FRAME_BUFFER_SIZE,
        &_frameLen,
        (const unsigned char*)b64.c_str(),
        b64.length()
    );

    if (ret != 0) {
        Serial.printf("[AI] Base64 decode error: %d\n", ret);
        _frameLen = 0;
        return false;
    }

    _hasNewFrame = true;
    return true;
}