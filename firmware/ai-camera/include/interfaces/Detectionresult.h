#pragma once
#include <Arduino.h>

// =============================================================================
// DetectionResult — shared between AIInference, I2CBridge and DetectionPipeline
// Lives in its own header so I2CBridge does NOT need to pull in AIInference.h
// (and therefore does NOT need Seeed_Arduino_SSCMA.h)
// =============================================================================
struct DetectionResult {
    bool    detected    = false;
    String  label       = "";
    float   confidence  = 0.0f;
    int     x = 0, y = 0;
    int     w = 0, h = 0;
};