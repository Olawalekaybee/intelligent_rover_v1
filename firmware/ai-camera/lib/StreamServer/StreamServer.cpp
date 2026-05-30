#include "StreamServer.h"
#include "config/AppConfig.h"

// Static members
DetectionInfo StreamServer::_detection;

// =============================================================================
// notifyDetection — called from DetectionPipeline on every confirmed detection
// =============================================================================
void StreamServer::notifyDetection(const char *label, uint8_t confidence, uint32_t uptimeS) {
    _detection.active     = true;
    _detection.confidence = confidence;
    _detection.uptimeS    = uptimeS;
    strncpy(_detection.label, label, sizeof(_detection.label) - 1);
    _detection.label[sizeof(_detection.label) - 1] = '\0';
}

// =============================================================================
// Viewer page
// =============================================================================
const char StreamServer::INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Intelligent Rover</title>
  <style>
    *{box-sizing:border-box;margin:0;padding:0}
    body{background:#0a0a0a;color:#d0d0d0;font-family:'Courier New',monospace;
         display:flex;flex-direction:column;align-items:center;padding:12px;min-height:100vh}

    /* ── Header ── */
    .header{text-align:center;margin-bottom:12px}
    .header h1{font-size:17px;color:#00d4aa;letter-spacing:3px;font-weight:normal}
    .header p{font-size:10px;color:#444;margin-top:3px;letter-spacing:1px}

    /* ── Stream wrapper ── */
    .stream-wrap{position:relative;width:100%;max-width:800px;
                 background:#0d0d0d;border:1px solid #1a3a30;border-radius:8px;
                 overflow:hidden;margin-bottom:8px}
    #stream{width:100%;display:block;min-height:200px}

    /* Top-left overlay: LIVE badge + fps */
    .ov-top{position:absolute;top:8px;left:8px;right:8px;
            display:flex;justify-content:space-between;align-items:center;
            pointer-events:none}
    .badge-live{background:rgba(0,212,170,.12);border:1px solid #00d4aa;
                color:#00d4aa;font-size:10px;padding:3px 10px;border-radius:3px;
                letter-spacing:1px}
    .badge-fps{background:rgba(0,0,0,.65);color:#888;
               font-size:10px;padding:3px 10px;border-radius:3px}

    /* Bottom overlay: detection alert */
    .ov-bot{position:absolute;bottom:0;left:0;right:0;padding:10px;
            background:linear-gradient(transparent,rgba(0,0,0,.75));
            display:flex;align-items:center;gap:10px;
            transition:opacity .3s;opacity:0;pointer-events:none}
    .ov-bot.visible{opacity:1}
    .det-icon{font-size:22px}
    .det-info{flex:1}
    .det-label{font-size:14px;color:#00ff88;font-weight:bold;letter-spacing:1px;
               text-transform:uppercase}
    .det-conf{font-size:11px;color:#aaa;margin-top:2px}
    .conf-bar{height:4px;background:#1a1a1a;border-radius:2px;margin-top:4px;
              overflow:hidden;width:120px}
    .conf-fill{height:100%;background:linear-gradient(90deg,#00d4aa,#00ff88);
               border-radius:2px;transition:width .5s ease}

    /* ── Stats grid ── */
    .stats{display:flex;gap:6px;width:100%;max-width:800px;
           margin-bottom:8px;flex-wrap:wrap}
    .stat{flex:1;min-width:80px;background:#0f0f0f;border:1px solid #1a1a1a;
          border-radius:6px;padding:8px 10px;text-align:center}
    .stat-val{font-size:18px;color:#00d4aa;font-weight:bold;line-height:1.2}
    .stat-lbl{font-size:9px;color:#444;letter-spacing:1px;margin-top:2px}
    .stat.warn .stat-val{color:#f0a500}
    .stat.alert .stat-val{color:#e05050}

    /* ── Detection panel ── */
    .det-panel{width:100%;max-width:800px;background:#0f0f0f;
               border:1px solid #1a1a1a;border-radius:8px;padding:12px;
               margin-bottom:8px}
    .det-panel h3{font-size:10px;color:#444;letter-spacing:2px;
                  text-transform:uppercase;margin-bottom:10px}
    .det-current{display:flex;align-items:center;gap:12px;padding:8px;
                 background:#0a0a0a;border-radius:6px;border:1px solid #1a3a30;
                 margin-bottom:8px}
    .det-dot{width:10px;height:10px;border-radius:50%;background:#333;
             flex-shrink:0;transition:background .3s}
    .det-dot.active{background:#00ff88;box-shadow:0 0 8px #00ff88}
    .det-text{flex:1}
    .det-main{font-size:13px;color:#ccc}
    .det-main span{color:#00d4aa;font-weight:bold}
    .det-sub{font-size:10px;color:#444;margin-top:2px}
    .det-time{font-size:10px;color:#555;text-align:right;min-width:60px}

    /* Detection history log */
    .det-log{max-height:100px;overflow-y:auto}
    .det-log-item{font-size:10px;color:#555;padding:3px 0;
                  border-bottom:1px solid #111;display:flex;gap:8px}
    .det-log-item span.lbl{color:#00d4aa}
    .det-log-item span.conf{color:#888}
    .det-log::-webkit-scrollbar{width:3px}
    .det-log::-webkit-scrollbar-thumb{background:#222;border-radius:2px}

    /* ── Controls ── */
    .ctrl{display:flex;gap:8px;flex-wrap:wrap;justify-content:center;
          margin-bottom:8px}
    a.btn{background:#0f0f0f;color:#00d4aa;border:1px solid #00d4aa;
          padding:7px 16px;border-radius:4px;font-family:inherit;
          font-size:11px;text-decoration:none;letter-spacing:1px;
          transition:background .2s}
    a.btn:hover{background:#00d4aa15}
  </style>
</head>
<body>
  <div class="header">
    <h1>INTELLIGENT ROVER</h1>
    <p>XIAO ESP32-S3 &middot; OV2640 &middot; GROVE VISION AI V2</p>
  </div>

  <!-- ── Live stream ── -->
  <div class="stream-wrap">
    <img id="stream" alt="Connecting...">
    <div class="ov-top">
      <span class="badge-live" id="badge">&#9679; LIVE</span>
      <span class="badge-fps" id="fps">-- fps</span>
    </div>
    <div class="ov-bot" id="det-overlay">
      <div class="det-icon">&#128100;</div>
      <div class="det-info">
        <div class="det-label" id="ov-label">PERSON</div>
        <div class="det-conf" id="ov-conf">Confidence: --</div>
        <div class="conf-bar"><div class="conf-fill" id="ov-bar"></div></div>
      </div>
    </div>
  </div>

  <!-- ── Stats ── -->
  <div class="stats">
    <div class="stat" id="st-fps">
      <div class="stat-val" id="sv-fps">--</div>
      <div class="stat-lbl">FPS</div>
    </div>
    <div class="stat" id="st-rssi">
      <div class="stat-val" id="sv-rssi">--</div>
      <div class="stat-lbl">RSSI dBm</div>
    </div>
    <div class="stat">
      <div class="stat-val" id="sv-heap">--</div>
      <div class="stat-lbl">HEAP KB</div>
    </div>
    <div class="stat">
      <div class="stat-val" id="sv-up">--</div>
      <div class="stat-lbl">UPTIME s</div>
    </div>
    <div class="stat">
      <div class="stat-val" id="sv-det">--</div>
      <div class="stat-lbl">LAST DET</div>
    </div>
  </div>

  <!-- ── Detection panel ── -->
  <div class="det-panel">
    <h3>&#127756; AI Detection — Grove Vision AI V2</h3>
    <div class="det-current">
      <div class="det-dot" id="det-dot"></div>
      <div class="det-text">
        <div class="det-main" id="det-main">Waiting for detection...</div>
        <div class="det-sub" id="det-sub">Person detection model &middot; 320&times;240</div>
      </div>
      <div class="det-time" id="det-elapsed">--</div>
    </div>
    <div class="det-log" id="det-log"></div>
  </div>

  <!-- ── Controls ── -->
  <div class="ctrl">
    <a class="btn" id="snap-btn" href="#">&#128247; Snapshot</a>
    <a class="btn" href="/ai/capture" target="_blank">&#129302; AI Frame</a>
    <a class="btn" href="/status" target="_blank">&#128202; Status JSON</a>
  </div>

  <script>
    var host      = window.location.hostname;
    var streamUrl = 'http://' + host + ':81/stream';
    var capUrl    = 'http://' + host + ':81/capture';

    document.getElementById('stream').src  = streamUrl;
    document.getElementById('snap-btn').href = capUrl;

    // ── FPS counter ──
    var fc = 0, t0 = Date.now();
    document.getElementById('stream').addEventListener('load', function() {
      fc++;
      var elapsed = (Date.now() - t0) / 1000;
      if (elapsed > 0 && fc % 5 === 0) {
        var fps = (fc / elapsed).toFixed(1);
        document.getElementById('fps').textContent    = fps + ' fps';
        document.getElementById('sv-fps').textContent = fps;
        var fEl = document.getElementById('st-fps');
        fEl.className = 'stat' + (fps < 2 ? ' warn' : '');
      }
    });

    // ── Detection history log ──
    var detHistory = [];
    function addToLog(label, conf, ago) {
      detHistory.unshift({label, conf, ago: new Date().toLocaleTimeString()});
      if (detHistory.length > 8) detHistory.pop();
      var html = detHistory.map(function(d) {
        return '<div class="det-log-item">' +
          '<span>' + d.ago + '</span>' +
          '<span class="lbl">' + d.label.toUpperCase() + '</span>' +
          '<span class="conf">' + d.conf + '%</span>' +
        '</div>';
      }).join('');
      document.getElementById('det-log').innerHTML = html;
    }

    var lastDetUptime = 0;
    var lastDetLabel  = '';
    var lastDetConf   = 0;

    // ── Status polling ──
    async function updateStatus() {
      try {
        var r = await fetch('/status');
        var d = await r.json();
        var now = d.uptime;

        // WiFi stats
        document.getElementById('sv-rssi').textContent = d.rssi;
        document.getElementById('sv-heap').textContent = Math.round(d.free_heap/1024);
        document.getElementById('sv-up').textContent   = d.uptime;

        var rssiEl = document.getElementById('st-rssi');
        rssiEl.className = 'stat' + (d.rssi < -80 ? ' alert' : d.rssi < -70 ? ' warn' : '');

        // Detection state
        var det = d.detection;
        var age = now - det.timestamp;  // seconds since last detection

        if (det.active && age < 5) {
          // Fresh detection — show overlays
          document.getElementById('det-overlay').classList.add('visible');
          document.getElementById('det-dot').classList.add('active');
          document.getElementById('ov-label').textContent = det.label.toUpperCase();
          document.getElementById('ov-conf').textContent  = 'Confidence: ' + det.confidence + '%';
          document.getElementById('ov-bar').style.width   = det.confidence + '%';
          document.getElementById('det-main').innerHTML   =
            'Detected: <span>' + det.label.toUpperCase() + '</span> — ' + det.confidence + '%';
          document.getElementById('sv-det').textContent   = age + 's ago';
        } else {
          document.getElementById('det-overlay').classList.remove('visible');
          document.getElementById('det-dot').classList.remove('active');
          if (det.active) {
            document.getElementById('det-main').innerHTML =
              'Last: <span>' + det.label + '</span> — ' + det.confidence + '%';
            document.getElementById('sv-det').textContent = age + 's ago';
          } else {
            document.getElementById('det-main').textContent = 'No detection';
            document.getElementById('sv-det').textContent = '--';
          }
        }

        // Add to log when new detection arrives
        if (det.active && det.timestamp !== lastDetUptime) {
          lastDetUptime = det.timestamp;
          lastDetLabel  = det.label;
          lastDetConf   = det.confidence;
          addToLog(det.label, det.confidence, age);
        }

        document.getElementById('det-elapsed').textContent =
          det.active ? age + 's ago' : '--';

      } catch(e) {
        document.getElementById('badge').textContent = 'OFFLINE';
        document.getElementById('badge').style.color = '#e05050';
      }
    }

    setInterval(updateStatus, 1000);
    updateStatus();
  </script>
</body>
</html>
)HTML";

// =============================================================================
void StreamServer::begin(uint16_t port) {
    if (_mutex) return;   // already started — prevent double-init
    _mutex = xSemaphoreCreateMutex();

    if (psramFound()) {
        _buf[0] = (uint8_t*)ps_malloc(MAX_FRAME_BUFFER_SIZE);
        _buf[1] = (uint8_t*)ps_malloc(MAX_FRAME_BUFFER_SIZE);
    }

    _server.on("/",           [this]() { handleRoot();      });
    _server.on("/ai/capture", [this]() { handleAICapture(); });
    _server.on("/status",     [this]() { handleStatus();    });
    _server.onNotFound([this]() { _server.send(404, "text/plain", "Not found"); });

    _server.begin();
    Serial.printf("[STREAM] Server started on port %d  IP: %s\n",
                  port, WiFi.localIP().toString().c_str());
}

void StreamServer::handle() { _server.handleClient(); }

void StreamServer::pushFrame(const uint8_t *data, size_t len) {
    if (!data || len == 0 || len > MAX_FRAME_BUFFER_SIZE || !_buf[_writeIdx]) return;
    memcpy(_buf[_writeIdx], data, len);
    _len[_writeIdx] = len;
    xSemaphoreTake(_mutex, portMAX_DELAY);
    uint8_t tmp  = _readIdx; _readIdx = _writeIdx; _writeIdx = tmp;
    xSemaphoreGive(_mutex);
}

bool StreamServer::hasClients() const { return _clientCount > 0; }

// =============================================================================
void StreamServer::handleRoot() { _server.send_P(200, "text/html", INDEX_HTML); }

void StreamServer::handleAICapture() {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    size_t   len = _len[_readIdx];
    uint8_t *buf = _buf[_readIdx];
    xSemaphoreGive(_mutex);

    if (!buf || len == 0) {
        _server.send(503, "text/plain", "No AI frame yet — Grove Vision AI V2 may not be connected");
        return;
    }
    _clientCount++;
    _server.sendHeader("Content-Disposition", "inline; filename=ai_frame.jpg");
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send_P(200, "image/jpeg", (const char*)buf, len);
    _clientCount--;
}

void StreamServer::handleStatus() {
    uint32_t now  = millis() / 1000;
    uint32_t age  = (_detection.uptimeS > 0) ? (now - _detection.uptimeS) : 9999;

    char json[384];
    snprintf(json, sizeof(json),
        "{"
        "\"ip\":\"%s\","
        "\"rssi\":%d,"
        "\"uptime\":%lu,"
        "\"wifi\":%s,"
        "\"ai_ready\":true,"
        "\"stream_clients\":%d,"
        "\"free_heap\":%lu,"
        "\"detection\":{"
            "\"active\":%s,"
            "\"label\":\"%s\","
            "\"confidence\":%u,"
            "\"timestamp\":%lu,"
            "\"age_s\":%lu"
        "}"
        "}",
        WiFi.localIP().toString().c_str(),
        WiFi.RSSI(),
        now,
        (WiFi.status() == WL_CONNECTED) ? "true" : "false",
        _clientCount,
        ESP.getFreeHeap(),
        _detection.active ? "true" : "false",
        _detection.label,
        _detection.confidence,
        _detection.uptimeS,
        age
    );

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", json);
}