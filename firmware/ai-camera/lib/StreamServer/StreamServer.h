#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// =============================================================================
// StreamServer — HTTP server on port 80
//
// Endpoints:
//   GET /           → full telemetry viewer (MJPEG + detection overlay)
//   GET /ai/capture → single JPEG from SSCMA (AI annotated frame)
//   GET /status     → JSON: WiFi, heap, uptime, detection state, fps
//
// Detection state is updated via notifyDetection() from DetectionPipeline
// and included in every /status response so the viewer overlay stays live.
// =============================================================================

struct DetectionInfo {
    bool     active     = false;
    char     label[16]  = {};
    uint8_t  confidence = 0;   // 0–100
    uint32_t uptimeS    = 0;   // millis()/1000 when detected
};

class StreamServer {
public:
    void begin(uint16_t port = 80);
    void handle();
    void pushFrame(const uint8_t *data, size_t len);
    bool hasClients() const;

    // Called from DetectionPipeline::onDetection()
    static void notifyDetection(const char *label, uint8_t confidence, uint32_t uptimeS);

private:
    WebServer         _server{80};
    SemaphoreHandle_t _mutex = nullptr;

    uint8_t* _buf[2]   = {nullptr, nullptr};
    size_t   _len[2]   = {0, 0};
    uint8_t  _writeIdx = 0;
    uint8_t  _readIdx  = 1;

    volatile int _clientCount = 0;

    static DetectionInfo _detection;
    static const char    INDEX_HTML[] PROGMEM;

    void handleRoot();
    void handleAICapture();
    void handleStatus();
};