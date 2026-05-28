#pragma once

#define DEVICE_NAME "Intelligent-Rover-CYD"
#define SERIAL_BAUD_RATE 115200

#define CYD_I2C_ADDRESS 0x42

// CYD I2C pins. Connect ESP32 main SDA/SCL to these physical pins.
// Main ESP32 GPIO21 SDA -> CYD GPIO27 SDA
// Main ESP32 GPIO22 SCL -> CYD GPIO22 SCL
#define CYD_I2C_SDA 27
#define CYD_I2C_SCL 22
#define CYD_I2C_FREQ 400000

#define OTA_HOSTNAME "rover-cyd"
#define OTA_PASSWORD "rover-ota-2025"

#define SCREEN_REFRESH_MS 500
#define PACKET_TIMEOUT_MS 5000
