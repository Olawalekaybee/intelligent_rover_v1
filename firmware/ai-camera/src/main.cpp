// =============================================================================
// Intelligent Rover — AI Camera Node  v1.5.0
// Board  : Seeed XIAO ESP32-S3 Sense
//
// Cameras:
//   OV2640 (XIAO built-in camera slot) → real-time MJPEG at 10-20fps
//   P5V04A Sunny → Grove Vision AI V2 → SSCMA I2C → AI detection events
//
// Network:
//   GET /          → live viewer (MJPEG from OV2640)
//   GET /stream    → raw MJPEG feed (OV2640, 10-20fps)
//   GET /capture   → single JPEG snapshot (OV2640)
//   GET /ai/capture → single JPEG with AI bounding boxes (SSCMA, ~1fps)
//   GET /status    → JSON system status
// =============================================================================

#include <Arduino.h>
#include <WebServer.h>
#include "config/AppConfig.h"

#include "interfaces/AIInference.h"
#include "interfaces/DetectionPipeline.h"
#include "interfaces/WiFiBridge.h"
#include "I2CBridge.h"
#include "StreamServer.h"
#include "CameraStream.h"

// =============================================================================
// Subsystem instances
// =============================================================================
AIInference       aiInference;
I2CBridge         i2cBridge;
WiFiBridge        wifiBridge;
DetectionPipeline detectionPipeline;
StreamServer      streamServer;    // serves HTML + status + AI frames
CameraStream      cameraStream;    // OV2640 real-time MJPEG

// =============================================================================
// Task declarations
// =============================================================================
void TaskDetectionPipeline(void *pvParameters);
void TaskCameraServer     (void *pvParameters);
void TaskAIFrameServer    (void *pvParameters);
void TaskWiFiBridge       (void *pvParameters);
void TaskStatusLED        (void *pvParameters);

// Dedicated WebServer for OV2640 camera stream
static WebServer cameraServer(81);  // port 81 — avoids conflicts with StreamServer port 80

// =============================================================================
// setup()
// =============================================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println();
    Serial.println("============================================");
    Serial.println("  Intelligent Rover — AI Camera Node v1.5");
    Serial.println("  OV2640: real-time MJPEG stream (port 81)");
    Serial.println("  Grove AI V2: person detection");
    Serial.println("============================================");
    Serial.printf("[SYSTEM] PSRAM: %s (%lu bytes)\n",
                  psramFound() ? "found" : "NOT found", ESP.getFreePsram());

    pinMode(STATUS_LED_PIN, OUTPUT);

    // Init order: OV2640 first (separate peripheral), then AI (I2C Wire0),
    // then I2C bridge (Wire1), then WiFi
    cameraStream.begin();
    bool aiOk = aiInference.begin();
    i2cBridge.begin();
    wifiBridge.begin();
    detectionPipeline.begin(&aiInference, &i2cBridge, &wifiBridge);

    if (!aiOk) {
        Serial.println("[SYSTEM] Grove Vision AI V2 not detected — AI detection disabled");
    }
    if (!cameraStream.isReady()) {
        Serial.println("[SYSTEM] OV2640 not detected — live stream disabled");
        Serial.println("[SYSTEM] Check camera module is seated in XIAO Sense slot");
    }

    xTaskCreatePinnedToCore(TaskDetectionPipeline, "AI_Pipeline",  8192, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(TaskCameraServer,      "Cam_Server",   8192, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(TaskAIFrameServer,     "AI_Frame_Srv", 4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(TaskWiFiBridge,        "WiFi_Bridge",  4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(TaskStatusLED,         "Status_LED",   2048, nullptr, 1, nullptr, 0);

    Serial.printf("[SYSTEM] AI camera node ready  heap=%lu B\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }

// =============================================================================
// TaskDetectionPipeline — Core 1, Priority 3
// AI detection via SSCMA. Captures images only when /ai/capture is requested.
// =============================================================================
void TaskDetectionPipeline(void *pvParameters) {
    while (true) {
        bool needFrame = streamServer.hasClients();  // true when /ai/capture is pending
        DetectionResult result = aiInference.update(needFrame);

        if (needFrame && aiInference.hasNewFrame()) {
            streamServer.pushFrame(aiInference.getFrameData(), aiInference.getFrameLen());
            aiInference.clearFrame();
        }

        if (result.detected) detectionPipeline.onDetection(result);
        detectionPipeline.tickHeartbeat();

        vTaskDelay(pdMS_TO_TICKS(INFERENCE_INTERVAL_MS));
    }
}

// =============================================================================
// TaskCameraServer — Core 0, Priority 2
// Serves OV2640 real-time MJPEG on port 81
// =============================================================================
void TaskCameraServer(void *pvParameters) {
    bool serverStarted = false;
    bool streamServerOk  = false;

    while (true) {
        if (wifiBridge.isConnected()) {
            if (!serverStarted) {
                // /stream — real-time MJPEG from OV2640
                cameraServer.on("/stream", HTTP_GET, []() {
                    if (!cameraStream.isReady()) {
                        cameraServer.send(503, "text/plain", "Camera not ready");
                        return;
                    }
                    WiFiClient client = cameraServer.client();
                    Serial.printf("[CAM] Stream client connected  heap=%lu\n", ESP.getFreeHeap());
                    cameraStream.streamToClient(client);
                    Serial.println("[CAM] Stream client disconnected");
                });

                // /capture — single JPEG from OV2640
                cameraServer.on("/capture", HTTP_GET, []() {
                    if (!cameraStream.isReady()) {
                        cameraServer.send(503, "text/plain", "Camera not ready");
                        return;
                    }
                    WiFiClient client = cameraServer.client();
                    cameraStream.serveSingleFrame(client);
                });

                cameraServer.begin(81);
                serverStarted = true;

                Serial.printf("[CAM] Server started — stream at http://%s:81/stream\n",
                              WiFi.localIP().toString().c_str());
            }
            cameraServer.handleClient();
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// =============================================================================
// TaskAIFrameServer — Core 0, Priority 2
// Serves the StreamServer: HTML viewer, status JSON, and SSCMA AI frames
// =============================================================================
void TaskAIFrameServer(void *pvParameters) {
    bool started = false;
    while (true) {
        if (wifiBridge.isConnected()) {
            if (!started) {
                streamServer.begin(STREAM_SERVER_PORT);
                started = true;
            }
            streamServer.handle();
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// =============================================================================
// TaskWiFiBridge — Core 0, Priority 2
// =============================================================================
void TaskWiFiBridge(void *pvParameters) {
    while (true) {
        wifiBridge.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// =============================================================================
// TaskStatusLED — Core 0, Priority 1
// =============================================================================
void TaskStatusLED(void *pvParameters) {
    while (true) {
        bool wifi = wifiBridge.isConnected();
        bool ai   = aiInference.isReady();
        bool cam  = cameraStream.isReady();

        uint32_t on, off;
        if      (wifi && ai && cam) { on = 100;  off = 1900; }
        else if (wifi && cam)       { on = 200;  off = 800;  }
        else if (wifi)              { on = 500;  off = 500;  }
        else                        { on = 200;  off = 200;  }

        digitalWrite(STATUS_LED_PIN, HIGH); vTaskDelay(pdMS_TO_TICKS(on));
        digitalWrite(STATUS_LED_PIN, LOW);  vTaskDelay(pdMS_TO_TICKS(off));
    }
}