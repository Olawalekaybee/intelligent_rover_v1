#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// =============================================================================
// StreamServer — MJPEG HTTP streaming server for Grove Vision AI V2 frames
//
// Endpoints:
//   GET /         → live viewer web page (works in any browser)
//   GET /stream   → MJPEG stream (multipart/x-mixed-replace)
//   GET /capture  → single JPEG snapshot (save or open in browser)
//   GET /status   → JSON system status
//
// Usage:
//   1. StreamServer::begin() — starts HTTP server
//   2. StreamServer::pushFrame(data, len) — called by AI pipeline with each JPEG
//   3. StreamServer::handle() — call from dedicated RTOS task
//
// Frame buffering:
//   Double buffer (A/B) in PSRAM.  AI pipeline writes to one while HTTP
//   clients read from the other.  A mutex protects the swap.
// =============================================================================
class StreamServer {
public:
    void begin(uint16_t port = 80);
    void handle();

    // Push a new JPEG frame — called from TaskDetectionPipeline
    void pushFrame(const uint8_t *data, size_t len);

    bool hasClients() const { return _clientCount > 0; }

private:
    WebServer         _server{80};
    SemaphoreHandle_t _mutex = nullptr;

    // Double buffer — one for writing, one for serving
    uint8_t* _buf[2]    = {nullptr, nullptr};
    size_t   _len[2]    = {0, 0};
    uint8_t  _writeIdx  = 0;
    uint8_t  _readIdx   = 1;

    volatile int _clientCount = 0;

    // Web page served at GET /
    static const char INDEX_HTML[] PROGMEM;

    void handleRoot();
    void handleStream();
    void handleCapture();
    void handleStatus();
};