// =============================================================================
// Intelligent Rover — AI Camera Node  v1.4.0
// Board  : Seeed XIAO ESP32-S3 Sense
// AI     : Grove Vision AI V2 (Himax WE2) + P5V04A Sunny camera
// Comms  : Wire0 (GPIO5/6)  → Grove Vision AI V2 (SSCMA)
//          Wire1 (GPIO3/4)  → I2C slave to ESP32 main at 0x55
//          WiFi             → MJPEG stream + OTA
// =============================================================================

#include <Arduino.h>
#include "config/AppConfig.h"

#include "interfaces/AIInference.h"
#include "interfaces/DetectionPipeline.h"
#include "interfaces/WiFiBridge.h"
#include "I2CBridge.h"
#include "StreamServer.h"

// =============================================================================
// Subsystem instances
// =============================================================================
AIInference       aiInference;
I2CBridge         i2cBridge;
WiFiBridge        wifiBridge;
DetectionPipeline detectionPipeline;
StreamServer      streamServer;

// =============================================================================
// Task declarations
// =============================================================================
void TaskDetectionPipeline(void *pvParameters);
void TaskStreamServer     (void *pvParameters);
void TaskWiFiBridge       (void *pvParameters);
void TaskStatusLED        (void *pvParameters);

// =============================================================================
// setup()
// =============================================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println();
    Serial.println("============================================");
    Serial.println("  Intelligent Rover — AI Camera Node");
    Serial.println("  XIAO ESP32-S3 + Grove Vision AI V2");
    Serial.println("  Camera: P5V04A Sunny");
    Serial.println("  Bridge: I2C slave 0x55 (GPIO3/4)");
    Serial.println("============================================");
    Serial.printf("[SYSTEM] PSRAM: %s (%lu bytes)\n",
                  psramFound() ? "found" : "NOT found",
                  ESP.getFreePsram());

    pinMode(STATUS_LED_PIN, OUTPUT);

    // Init order: AI (Wire0) → I2C bridge (Wire1) → WiFi → pipeline
    bool aiOk = aiInference.begin();
    i2cBridge.begin();
    wifiBridge.begin();
    detectionPipeline.begin(&aiInference, &i2cBridge, &wifiBridge);

    if (wifiBridge.isConnected()) {
        streamServer.begin(STREAM_SERVER_PORT);
    }

    if (!aiOk) {
        Serial.println("[SYSTEM] WARNING: Grove Vision AI V2 not detected");
        Serial.println("[SYSTEM] Check: SDA=GPIO5  SCL=GPIO6");
    }

    xTaskCreatePinnedToCore(TaskDetectionPipeline, "AI_Pipeline",
                            8192, nullptr, 3, nullptr, 1);
    xTaskCreatePinnedToCore(TaskStreamServer,      "Stream_Srv",
                            8192, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(TaskWiFiBridge,        "WiFi_Bridge",
                            4096, nullptr, 2, nullptr, 0);
    xTaskCreatePinnedToCore(TaskStatusLED,         "Status_LED",
                            2048, nullptr, 1, nullptr, 0);

    Serial.println("[SYSTEM] AI camera node ready");
    Serial.printf("[SYSTEM] Free heap: %lu B\n", ESP.getFreeHeap());
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }

// =============================================================================
// TaskDetectionPipeline — Core 1, Priority 3
// =============================================================================
void TaskDetectionPipeline(void *pvParameters) {
    while (true) {
        // Always capture frames so /capture endpoint always has fresh data.
        // hasClients() only tracks MJPEG clients; periodic /capture requests
        // are one-shot and never increment the client count.
        bool needFrame = aiInference.isReady();
        DetectionResult result = aiInference.update(needFrame);

        if (needFrame && aiInference.hasNewFrame()) {
            streamServer.pushFrame(aiInference.getFrameData(),
                                   aiInference.getFrameLen());
            aiInference.clearFrame();
        }

        if (result.detected) {
            detectionPipeline.onDetection(result);
        }
        detectionPipeline.tickHeartbeat();

        vTaskDelay(pdMS_TO_TICKS(INFERENCE_INTERVAL_MS));
    }
}

// =============================================================================
// TaskStreamServer — Core 0, Priority 2
// =============================================================================
void TaskStreamServer(void *pvParameters) {
    while (true) {
        if (wifiBridge.isConnected()) streamServer.handle();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// =============================================================================
// TaskWiFiBridge — Core 0, Priority 2
// =============================================================================
void TaskWiFiBridge(void *pvParameters) {
    bool streamStarted = false;
    while (true) {
        wifiBridge.update();
        if (wifiBridge.isConnected() && !streamStarted) {
            streamStarted = true;
            streamServer.begin(STREAM_SERVER_PORT);
        }
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

        uint32_t on  = wifi && ai ? 100  : wifi ? 200  : 200;
        uint32_t off = wifi && ai ? 1900 : wifi ? 800  : 200;

        digitalWrite(STATUS_LED_PIN, HIGH); vTaskDelay(pdMS_TO_TICKS(on));
        digitalWrite(STATUS_LED_PIN, LOW);  vTaskDelay(pdMS_TO_TICKS(off));
    }
}