#include "StreamServer.h"
#include "config/AppConfig.h"

// =============================================================================
// Viewer page — served at GET /
// Simple auto-refreshing MJPEG viewer that works on any device on the network.
// =============================================================================
const char StreamServer::INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Intelligent Rover — Live Camera</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{background:#0d0d0d;color:#e0e0e0;font-family:'Courier New',monospace;
         display:flex;flex-direction:column;align-items:center;padding:16px;min-height:100vh}
    h1{font-size:18px;color:#00d4aa;margin:12px 0 4px;letter-spacing:2px}
    .sub{font-size:11px;color:#555;margin-bottom:14px}
    .wrap{position:relative;width:100%;max-width:640px}
    #frame{width:100%;border:1px solid #1e3a2f;border-radius:6px;display:block;
           background:#111;min-height:240px;transition:opacity .2s}
    .badge{position:absolute;top:8px;left:8px;background:rgba(0,212,170,.15);
           border:1px solid #00d4aa;color:#00d4aa;font-size:10px;padding:2px 8px;
           border-radius:3px;letter-spacing:1px}
    .fps{position:absolute;top:8px;right:8px;background:rgba(0,0,0,.6);
         color:#aaa;font-size:10px;padding:2px 8px;border-radius:3px}
    .ctrl{display:flex;gap:10px;margin:12px 0;flex-wrap:wrap;justify-content:center}
    button,a.btn{background:#1a1a1a;color:#00d4aa;border:1px solid #00d4aa;
                 padding:6px 16px;border-radius:4px;cursor:pointer;
                 font-family:inherit;font-size:12px;text-decoration:none}
    button:hover,a.btn:hover{background:#00d4aa22}
    #info{font-size:11px;color:#555;margin:4px 0;min-height:16px}
  </style>
</head>
<body>
  <h1>INTELLIGENT ROVER</h1>
  <div class="sub">XIAO ESP32-S3 + Grove Vision AI V2 + P5V04A Sunny</div>
  <div class="wrap">
    <img id="frame" src="/capture?t=0" alt="Loading first frame...">
    <div class="badge" id="badge">● LIVE</div>
    <div class="fps" id="fps">-- fps</div>
  </div>
  <div id="info">Connecting...</div>
  <div class="ctrl">
    <a class="btn" href="/capture" target="_blank">📷 Snapshot</a>
    <button onclick="setRate(1000)">1fps</button>
    <button onclick="setRate(2000)">0.5fps</button>
    <button onclick="setRate(500)">2fps</button>
  </div>
  <script>
    var interval = 2000, timer, lastLoad = Date.now(), frameCount = 0;
    function refresh() {
      var img = document.getElementById('frame');
      var prev = Date.now();
      img.style.opacity = 0.7;
      img.onload = function() {
        img.style.opacity = 1;
        var dt = Date.now() - prev;
        document.getElementById('fps').textContent =
          (1000/dt).toFixed(1) + ' fps  (' + dt + 'ms)';
        frameCount++;
      };
      img.src = '/capture?t=' + Date.now();
    }
    function setRate(ms) {
      interval = ms;
      clearInterval(timer);
      timer = setInterval(refresh, interval);
    }
    async function updateInfo() {
      try {
        var r = await fetch('/status');
        var d = await r.json();
        document.getElementById('info').textContent =
          'IP: ' + d.ip + '  RSSI: ' + d.rssi + 'dBm' +
          '  Heap: ' + Math.round(d.free_heap/1024) + 'KB' +
          '  Uptime: ' + d.uptime + 's';
      } catch(e) {}
    }
    setRate(interval);
    setInterval(updateInfo, 5000);
    updateInfo();
  </script>
</body>
</html>
)HTML";

// =============================================================================
void StreamServer::begin(uint16_t port) {
    _mutex = xSemaphoreCreateMutex();

    // Allocate double frame buffers from PSRAM
    if (psramFound()) {
        _buf[0] = (uint8_t*)ps_malloc(MAX_FRAME_BUFFER_SIZE);
        _buf[1] = (uint8_t*)ps_malloc(MAX_FRAME_BUFFER_SIZE);
    }

    if (!_buf[0] || !_buf[1]) {
        Serial.println("[STREAM] Frame buffer alloc failed — streaming disabled");
    }

    _server.on("/",        [this]() { handleRoot();    });
    _server.on("/stream",  [this]() { handleStream();  });
    _server.on("/capture", [this]() { handleCapture(); });
    _server.on("/status",  [this]() { handleStatus();  });
    _server.onNotFound([this]() {
        _server.send(404, "text/plain", "Not found");
    });

    _server.begin();

    Serial.printf("[STREAM] HTTP server started on port %d\n", port);
    Serial.printf("[STREAM] View stream at: http://%s/\n",
                  WiFi.localIP().toString().c_str());
}

void StreamServer::handle() {
    _server.handleClient();
}

// =============================================================================
// pushFrame — called from AI pipeline task with each decoded JPEG
// Swaps write/read buffers atomically so the stream handler always
// serves a complete, consistent frame.
// =============================================================================
void StreamServer::pushFrame(const uint8_t *data, size_t len) {
    if (!data || len == 0 || !_buf[_writeIdx]) return;
    if (len > MAX_FRAME_BUFFER_SIZE) return;

    // Write into the inactive buffer
    memcpy(_buf[_writeIdx], data, len);
    _len[_writeIdx] = len;

    // Swap buffers under mutex
    xSemaphoreTake(_mutex, portMAX_DELAY);
    uint8_t tmp = _readIdx;
    _readIdx     = _writeIdx;
    _writeIdx    = tmp;
    xSemaphoreGive(_mutex);
}

// =============================================================================
// handleRoot — serves the viewer HTML page
// =============================================================================
void StreamServer::handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

// =============================================================================
// handleStream — keeps connection alive and pushes MJPEG frames
//
// This function BLOCKS until the client disconnects — run from a dedicated
// RTOS task (TaskStreamServer) so it doesn't starve other handlers.
// =============================================================================
void StreamServer::handleStream() {
    if (!_buf[0] || !_buf[1]) {
        _server.send(503, "text/plain", "Streaming unavailable (no PSRAM)");
        return;
    }

    WiFiClient client = _server.client();
    if (!client) return;

    // MJPEG multipart header
    client.print(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n"
        "Connection: close\r\n\r\n"
    );

    _clientCount++;
    Serial.printf("[STREAM] Client connected — total: %d\n", _clientCount);

    uint32_t lastFrameMs  = 0;

    while (client.connected()) {
        uint32_t now = millis();

        // Throttle to target FPS
        if ((now - lastFrameMs) < STREAM_FRAME_INTERVAL_MS) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        lastFrameMs = now;

        // Read the current frame under mutex
        xSemaphoreTake(_mutex, portMAX_DELAY);
        size_t   frameLen = _len[_readIdx];
        uint8_t *framePtr = _buf[_readIdx];
        xSemaphoreGive(_mutex);

        if (frameLen == 0) {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        // Send MJPEG boundary + headers + JPEG data
        client.printf(
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %u\r\n\r\n",
            (unsigned)frameLen
        );
        client.write(framePtr, frameLen);
        client.print("\r\n");
    }

    _clientCount--;
    Serial.printf("[STREAM] Client disconnected — total: %d\n", _clientCount);
}

// =============================================================================
// handleCapture — serves single JPEG snapshot (save / share)
// =============================================================================
void StreamServer::handleCapture() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    size_t   len = _len[_readIdx];
    uint8_t *buf = _buf[_readIdx];
    xSemaphoreGive(_mutex);

    if (!buf || len == 0) {
        _server.send(503, "text/plain", "No frame available yet");
        return;
    }

    _server.sendHeader("Content-Disposition",
                        "inline; filename=rover_capture.jpg");
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send_P(200, "image/jpeg", (const char*)buf, len);
}

// =============================================================================
// handleStatus — JSON status for the viewer page overlay
// =============================================================================
void StreamServer::handleStatus() {
    char json[256];
    snprintf(json, sizeof(json),
             "{\"ip\":\"%s\",\"rssi\":%d,\"uptime\":%lu,"
             "\"wifi\":%s,\"ai_ready\":true,\"stream_clients\":%d,"
             "\"free_heap\":%lu}",
             WiFi.localIP().toString().c_str(),
             WiFi.RSSI(),
             millis() / 1000,
             (WiFi.status() == WL_CONNECTED) ? "true" : "false",
             _clientCount,
             ESP.getFreeHeap());

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", json);
}