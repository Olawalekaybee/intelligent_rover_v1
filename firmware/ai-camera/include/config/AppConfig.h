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

// ── I2C Bridge to ESP32 Main Controller (Wire1 — separate from SSCMA bus) ─────
// XIAO GPIO3 (D2) ↔ ESP32 GPIO4  (SDA)
// XIAO GPIO4 (D3) ↔ ESP32 GPIO13 (SCL)
// 4.7kΩ pull-up resistors to 3.3V required on both lines
//
// NOTE: GPIO43/44 (D6/D7) are USB-Serial/JTAG pins on XIAO ESP32-S3.
//       Using them for UART causes the XIAO to freeze. I2C on GPIO3/4 avoids this.
#define AI_I2C_SLAVE_ADDR   0x55    // XIAO slave address (not 0x62=WE2, 0x38=AHT20, 0x53=ENS160)
#define AI_I2C1_SDA_PIN     3       // D2 on XIAO — Wire1 SDA
#define AI_I2C1_SCL_PIN     4       // D3 on XIAO — Wire1 SCL
#define AI_I2C1_FREQ        100000  // 100 kHz — safe for long wires

// ── Detection thresholds ──────────────────────────────────────────────────────
#define MIN_CONFIDENCE_SCORE    50    // 0-100 scale (Grove SSCMA uses 0-100)
#define INFERENCE_INTERVAL_MS   200   // 5 FPS inference rate
#define HEARTBEAT_INTERVAL_MS   10000 // 10s UART heartbeat to main ESP32
#define DETECTION_COOLDOWN_MS   1000  // min ms between UART sends for same detection

// ── WiFi ──────────────────────────────────────────────────────────────────────
#define WIFI_RECONNECT_INTERVAL_MS  15000
#define WIFI_CONNECT_TIMEOUT_MS     10000

// ── OTA ───────────────────────────────────────────────────────────────────────
#define OTA_HOSTNAME        "rover-camera"
#define OTA_PASSWORD        "rover-ota-2025"

// ── Status LED ────────────────────────────────────────────────────────────────
#define STATUS_LED_PIN      LED_BUILTIN
// ── MJPEG Stream Server ───────────────────────────────────────────────────────
#define STREAM_SERVER_PORT      80
#define STREAM_FPS_TARGET       5       // frames per second when client connected
#define STREAM_FRAME_INTERVAL_MS  (1000 / STREAM_FPS_TARGET)
#define MAX_FRAME_BUFFER_SIZE   (200 * 1024)   // 200 KB per frame buffer (PSRAM)