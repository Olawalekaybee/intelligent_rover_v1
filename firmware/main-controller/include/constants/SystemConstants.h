#pragma once

// =============================================================================
// RTOS Task Configuration — Intelligent Rover Main Controller
//
// All FreeRTOS task parameters live here so main.cpp contains zero magic
// numbers.  Change a value in one place; it propagates everywhere.
//
// ESP32 dual-core:
//   Core 0 = PRO_CPU  — runs the WiFi/BT stack; used for service tasks
//   Core 1 = APP_CPU  — free of radio stack; used for time-critical tasks
//
// FreeRTOS priority: higher number = runs first.
// ESP32 Arduino default configMAX_PRIORITIES = 25 (0–24 valid).
// =============================================================================

// ── Core assignments ──────────────────────────────────────────────────────────
#define CORE_SERVICES   0   // WiFi stack lives here; all background tasks
#define CORE_BT_MOTOR   1   // Bluetooth + motor — must not share with WiFi
#define CORE_AI         1   // AI UART reader — high priority, same as BT core

// ── Stack sizes (bytes) ───────────────────────────────────────────────────────
// Rule of thumb: increase by 512 if a task hits a stack-overflow panic.
#define STACK_BT_CONTROL        4096
#define STACK_OTA_SERVICE       8192    // ArduinoOTA uses ~6 KB internally
#define STACK_AI_EVENT_READ     4096
#define STACK_SENSOR_READ       6144    // increased: dtoa() float→string needs heap; fragmented by BT+WiFi
#define STACK_GPS_READ          4096
#define STACK_NETWORK_SERVICE   8192    // WiFi + HTTPClient needs headroom
#define STACK_DISPLAY_SERVICE   4096
#define STACK_STATUS_LED        2048

// ── Task priorities ───────────────────────────────────────────────────────────
// README task table reproduced here as the single source of truth:
//   Bluetooth Control  Core 1  Priority 4   rover movement control
//   AI Event Receiver  Core 1  Priority 3   detection processing
//   Sensor Read        Core 0  Priority 2   environmental telemetry
//   GPS Read           Core 0  Priority 2   GPS processing
//   Network Service    Core 0  Priority 2   WiFi + ThingSpeak
//   OTA Service        Core 0  Priority 6   OTA firmware updates (high so it
//                                           can preempt uploads safely)
//   Display Service    Core 0  Priority 1   CYD updates
//   Status LED         Core 0  Priority 1   heartbeat blink
#define PRIORITY_BT_CONTROL         4
#define PRIORITY_OTA_SERVICE        6
#define PRIORITY_AI_EVENT_READ      3
#define PRIORITY_SENSOR_READ        2
#define PRIORITY_GPS_READ           2
#define PRIORITY_NETWORK_SERVICE    2
#define PRIORITY_DISPLAY_SERVICE    1
#define PRIORITY_STATUS_LED         1