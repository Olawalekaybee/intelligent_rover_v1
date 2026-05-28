#pragma once

// ======================================================
// Device Identity
// ======================================================

#define DEVICE_NAME "Intelligent-Rover-Main"
#define SERIAL_BAUD_RATE 115200

// ======================================================
// Feature Flags
// Set disabled modules to 0 while hardware is not connected
// ======================================================

#define ENABLE_BLUETOOTH_CONTROL 1
#define ENABLE_WIFI_TELEMETRY    1
#define ENABLE_OTA_UPDATE        1
#define ENABLE_THINGSPEAK_UPLOAD 1

#define ENABLE_AHT20_SENSOR      1
#define ENABLE_MQ135_SENSOR      1
#define ENABLE_ENS160_SENSOR     0

#define ENABLE_GPS_MODULE        0
#define ENABLE_CYD_DISPLAY       0
#define ENABLE_BATTERY_MONITOR   0

#define ENABLE_AI_EVENT_RECEIVER 1

// ======================================================
// Motor PWM
// ======================================================

#define MOTOR_PWM_FREQ 20000
#define MOTOR_PWM_RESOLUTION 8

#define DEFAULT_MOTOR_SPEED 180
#define MAX_MOTOR_SPEED 255
#define MIN_MOTOR_SPEED 0

// ======================================================
// Control
// ======================================================

#define SPEED_MIN 60
#define SPEED_MAX 255
#define COMMAND_TIMEOUT_MS 500

// ======================================================
// Task Intervals
// ======================================================

#define SENSOR_READ_INTERVAL_MS 2000
#define GPS_READ_INTERVAL_MS 1000
#define AI_EVENT_READ_INTERVAL_MS 20
#define DISPLAY_UPDATE_INTERVAL_MS 1000
#define THINGSPEAK_UPLOAD_INTERVAL_MS 15000
#define SYSTEM_HEALTH_INTERVAL_MS 5000

// ======================================================
// Battery Voltage Divider
// batteryVoltage = adcVoltage * BATTERY_DIVIDER_RATIO * BATTERY_CALIBRATION
// ======================================================

#define ADC_REFERENCE_VOLTAGE 3.30f
#define ADC_MAX_COUNTS 4095.0f

// Example divider: 100k top + 27k bottom = 4.7037 ratio
#define BATTERY_DIVIDER_RATIO 4.7037f
#define BATTERY_CALIBRATION 1.00f

// ======================================================
// I2C Display Bridge
// ======================================================

#define CYD_I2C_ADDRESS 0x42

// ======================================================
// AI UART Event Protocol
// ======================================================

#define AI_UART_BAUD_RATE 115200
#define AI_EVENT_BUFFER_SIZE 256

// ======================================================
// MQ135 Gas Sensor
// ======================================================

#define MQ135_SAMPLE_COUNT      20
#define MQ135_WARMUP_TIME_MS    30000
#define MQ135_ADC_MIN_VALID     10
#define MQ135_ADC_MAX_VALID     4090