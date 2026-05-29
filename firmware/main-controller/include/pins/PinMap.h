#pragma once

// Shared I2C bus for ENS160 + AHT20 + CYD display bridge
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// GPS UART
#define PIN_GPS_RX 16
#define PIN_GPS_TX 17

// XIAO AI node I2C bridge (Wire1 — separate from sensor bus on Wire0)
// XIAO GPIO3 (D2) ↔ ESP32 GPIO4  (SDA1)
// XIAO GPIO4 (D3) ↔ ESP32 GPIO13 (SCL1)
// 4.7 kΩ pull-ups to 3.3 V required on both lines
#define PIN_AI_I2C_SDA  4    // Wire1 SDA — freed from old UART RX
#define PIN_AI_I2C_SCL  13   // Wire1 SCL — freed from old UART TX

// MQ-135 analog input
#define PIN_MQ135_ADC 34

// Battery voltage monitor
#define PIN_BATTERY_ADC 35

// Servo PWM
#define PIN_SERVO 23

// Left BTS7960 motor driver
#define PIN_LEFT_LPWM 25
#define PIN_LEFT_RPWM 26
#define PIN_LEFT_LEN  33
#define PIN_LEFT_REN  32

// Right BTS7960 motor driver
#define PIN_RIGHT_LPWM 27
#define PIN_RIGHT_RPWM 14
#define PIN_RIGHT_LEN  18
#define PIN_RIGHT_REN  19

// Status LED
#define PIN_STATUS_LED 2