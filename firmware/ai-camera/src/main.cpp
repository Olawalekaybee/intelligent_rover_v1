// =============================================================================
// Intelligent Rover — AI Camera Node
// Board  : Seeed XIAO ESP32-S3 Sense (no camera attachment)
// AI     : Grove Vision AI V2 (Himax WE2) + P5V04A Sunny camera
// Comms  : I2C (Grove → XIAO) + UART (XIAO → ESP32 main) + WiFi
// =============================================================================

#include <Arduino.h>
#include "config/AppConfig.h"

#include "interfaces/AIInference.h"
#include "interfaces/UARTBridge.h"
#include "interfaces/WiFiBridge.h"
#include "interfaces/DetectionPipeline.h"

// =============================================================================
// Subsystem instances
// =============================================================================
AIInference       aiInference;
UARTBridge        uartBridge;
WiFiBridge        wifiBridge;
DetectionPipeline detectionPipeline;

// =============================================================================
// RTOS task declarations
// =============================================================================
void TaskDetectionPipeline(void *pvParameters);
void TaskWiFiBridge(void *pvParameters);
void TaskStatusLED(void *pvParameters);

// =============================================================================
// setup()
// =============================================================================
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println();
    Serial.println("===========================================");
    Serial.println("  Intelligent Rover — AI Camera Node");
    Serial.println("  XIAO ESP32-S3 + Grove Vision AI V2");
    Serial.println("  Camera: P5V04A Sunny");
    Serial.println("===========================================");

    pinMode(STATUS_LED_PIN, OUTPUT);

    // Initialize in order: AI → UART → WiFi → Pipeline
    bool aiOk = aiInference.begin();
    uartBridge.begin();
    wifiBridge.begin();

    detectionPipeline.begin(&aiInference, &uartBridge, &wifiBridge);

    if (!aiOk) {
        Serial.println("[SYSTEM] WARNING: Grove Vision AI V2 not detected");
        Serial.println("[SYSTEM] Check I2C wiring: SDA=GPIO5, SCL=GPIO6");
        Serial.println("[SYSTEM] Continuing — will retry in detection loop");
    }

    // TaskDetectionPipeline on Core 1 (higher priority — time-sensitive)
    xTaskCreatePinnedToCore(
        TaskDetectionPipeline, "DetectionPipeline",
        8192, nullptr, 3, nullptr, 1);

    // TaskWiFiBridge on Core 0 (WiFi stack prefers Core 0)
    xTaskCreatePinnedToCore(
        TaskWiFiBridge, "WiFiBridge",
        4096, nullptr, 2, nullptr, 0);

    // TaskStatusLED on Core 0
    xTaskCreatePinnedToCore(
        TaskStatusLED, "StatusLED",
        2048, nullptr, 1, nullptr, 0);

    Serial.println("[SYSTEM] AI camera node ready");
    Serial.printf("[SYSTEM] Free heap: %lu bytes\n", ESP.getFreeHeap());
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// =============================================================================
// TaskDetectionPipeline — Core 1, Priority 3
// Polls Grove Vision AI V2 and routes results over UART + WiFi
// =============================================================================
void TaskDetectionPipeline(void *pvParameters) {
    while (true) {
        detectionPipeline.update();
        vTaskDelay(pdMS_TO_TICKS(INFERENCE_INTERVAL_MS));
    }
}

// =============================================================================
// TaskWiFiBridge — Core 0, Priority 2
// Maintains WiFi connection and handles OTA update requests
// =============================================================================
void TaskWiFiBridge(void *pvParameters) {
    while (true) {
        wifiBridge.update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// =============================================================================
// TaskStatusLED — Core 0, Priority 1
// Blink pattern:  WiFi connected = slow (900ms off / 100ms on)
//                 No WiFi        = fast (200ms on / 200ms off)
// =============================================================================
void TaskStatusLED(void *pvParameters) {
    while (true) {
        bool wifi = wifiBridge.isConnected();
        bool ai   = aiInference.isReady();

        if (wifi && ai) {
            // Both ready: slow heartbeat
            digitalWrite(STATUS_LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100));
            digitalWrite(STATUS_LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(1900));
        } else if (wifi && !ai) {
            // WiFi ok, AI not ready: medium blink
            digitalWrite(STATUS_LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(200));
            digitalWrite(STATUS_LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(800));
        } else {
            // No WiFi: fast blink
            digitalWrite(STATUS_LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(200));
            digitalWrite(STATUS_LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}