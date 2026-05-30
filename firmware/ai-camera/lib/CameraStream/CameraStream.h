#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

// =============================================================================
// CameraStream — OV2640 on XIAO ESP32-S3 Sense
//
// Uses the ESP32-S3 DVP camera peripheral (completely separate from I2C).
// Captures JPEG in hardware — no CPU-side encoding needed.
// Serves real-time MJPEG at /stream and single frames at /capture.
//
// Camera pins for XIAO ESP32-S3 Sense (Seeed official pinout):
//   XCLK=10  SDA=40  SCL=39
//   D0-D7=15,17,18,16,14,12,11,48
//   VSYNC=38  HREF=47  PCLK=13
// =============================================================================
class CameraStream {
public:
    bool begin();
    bool isReady() const { return _ready; }

    // Stream MJPEG to a connected WiFiClient until disconnect.
    // Call from a dedicated RTOS task — this function blocks.
    void streamToClient(WiFiClient &client);

    // Capture a single JPEG frame and write it as HTTP response.
    void serveSingleFrame(WiFiClient &client);

private:
    bool _ready = false;
};