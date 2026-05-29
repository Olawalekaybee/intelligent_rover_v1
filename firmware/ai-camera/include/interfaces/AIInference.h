#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Seeed_Arduino_SSCMA.h>

// =============================================================================
// DetectionResult — defined here so I2CBridge and DetectionPipeline can reuse
// via the DETECTION_RESULT_DEFINED guard without a separate header file
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
// AIInference — Grove Vision AI V2 (Himax WE2) via SSCMA I2C (Wire0)
// =============================================================================
class AIInference {
public:
    bool begin();
    DetectionResult update(bool captureFrame = false);
    bool isReady() const { return _ready; }

    const uint8_t* getFrameData() const { return _frameBuffer; }
    size_t         getFrameLen()  const { return _frameLen;    }
    bool           hasNewFrame()  const { return _hasNewFrame; }
    void           clearFrame()         { _hasNewFrame = false; }

private:
    SSCMA    _ai;
    bool     _ready       = false;
    uint32_t _lastRetryMs = 0;

    uint8_t* _frameBuffer = nullptr;
    size_t   _frameLen    = 0;
    bool     _hasNewFrame = false;

    static String classLabel(int target);
    bool   decodeFrame(const String &b64);
};