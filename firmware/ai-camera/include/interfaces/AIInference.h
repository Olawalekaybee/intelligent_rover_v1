#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Seeed_Arduino_SSCMA.h>

// Detection result passed through the pipeline
struct DetectionResult {
    bool    detected    = false;
    String  label       = "";
    float   confidence  = 0.0f;  // 0.0 – 1.0
    int     x           = 0;     // bounding box (pixels in Grove AI frame)
    int     y           = 0;
    int     w           = 0;
    int     h           = 0;
};

// =============================================================================
// AIInference — Grove Vision AI V2 (Himax WE2) via SSCMA I2C
//
// The Grove module runs the model internally.
// XIAO polls it via I2C and receives bounding-box / class results.
// =============================================================================
class AIInference {
public:
    bool begin();
    DetectionResult update();
    bool isReady() const { return _ready; }

private:
    SSCMA   _ai;
    bool    _ready = false;
    uint32_t _lastRetryMs = 0;

    // Maps Grove SSCMA class index to human-readable label
    // Adjust these to match the model loaded on your Grove Vision AI V2
    String classLabel(int target);
};