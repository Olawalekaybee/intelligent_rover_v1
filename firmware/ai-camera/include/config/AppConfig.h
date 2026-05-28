#pragma once

// =============================================================================
// AI Camera Node — App Configuration
// Board: Seeed XIAO ESP32-S3 Sense (no camera attachment used)
// AI Module: Grove Vision AI V2 (Himax WE2) + P5V04A Sunny camera
// =============================================================================

#define DEVICE_NAME         "Intelligent-Rover-AI-Camera"
#define SERIAL_BAUD_RATE    115200

// ── Grove Vision AI V2 (SSCMA) — I2C ─────────────────────────────────────────
// XIAO ESP32-S3 default Grove I2C pins
#define GROVE_SDA_PIN       5    // D4 on XIAO
#define GROVE_SCL_PIN       6    // D5 on XIAO
#define GROVE_I2C_FREQ      400000  // 400 kHz fast mode

// ── UART Bridge to ESP32 Main Controller ─────────────────────────────────────
// XIAO pin 43 (TX) → ESP32 UART2 RX
// XIAO pin 44 (RX) → ESP32 UART2 TX
#define AI_UART_TX_PIN      43
#define AI_UART_RX_PIN      44
#define AI_EVENT_UART_BAUD_RATE 115200

// ── Detection thresholds ──────────────────────────────────────────────────────
#define MIN_CONFIDENCE_SCORE    50    // 0-100 scale (Grove SSCMA uses 0-100)
#define INFERENCE_INTERVAL_MS   200   // 5 FPS inference rate
#define HEARTBEAT_INTERVAL_MS   10000 // 10s UART heartbeat to main ESP32

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_RECONNECT_INTERVAL_MS  15000
#define WIFI_CONNECT_TIMEOUT_MS     10000

// ── OTA ───────────────────────────────────────────────────────────────────────
#define OTA_HOSTNAME        "rover-camera"
#define OTA_PASSWORD        "rover-ota-2025"

// ── Status LED ────────────────────────────────────────────────────────────────
#define STATUS_LED_PIN      LED_BUILTIN