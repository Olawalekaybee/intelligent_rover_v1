// =============================================================================
// Intelligent Rover — ESP32 Main Controller
// Roles: rover movement, Bluetooth control, AHT20, MQ135,
//        ThingSpeak, OTA, optional GPS, AI-event reception, CYD display bridge.
// =============================================================================

#include <Arduino.h>
#include <WiFi.h>

#include "config/AppConfig.h"
#include "pins/PinMap.h"

#include "MotorControl.h"
#include "BluetoothControl.h"
#include "SensorManager.h"
#include "GPSManager.h"
#include "TelemetryData.h"
#include "RoverWiFiManager.h"
#include "OTAUpdate.h"
#include "ThingSpeakClient.h"
#include "DisplayBridge.h"
#include "AIEventReceiver.h"

MotorControl      motor;
BluetoothControl  bluetooth;
SensorManager     sensors;
GPSManager        gpsManager;
TelemetryData     telemetry;
RoverWiFiManager  wifiManager;
OTAUpdate         otaUpdate;
ThingSpeakClient  thingSpeakClient;
DisplayBridge     displayBridge;
AIEventReceiver   aiEventReceiver;

static volatile uint8_t currentSpeed = DEFAULT_MOTOR_SPEED;
static volatile uint32_t lastCommandTime = 0;

static SemaphoreHandle_t telemetryMutex = nullptr;

void TaskBluetoothControl(void *pvParameters);
void TaskSensorRead(void *pvParameters);
void TaskNetworkService(void *pvParameters);
void TaskStatusLED(void *pvParameters);

#if ENABLE_OTA_UPDATE
void TaskOTAService(void *pvParameters);
#endif

#if ENABLE_GPS_MODULE
void TaskGPSRead(void *pvParameters);
#endif

#if ENABLE_AI_EVENT_RECEIVER
void TaskAIEventRead(void *pvParameters);
#endif

#if ENABLE_CYD_DISPLAY
void TaskDisplayService(void *pvParameters);
#endif

template <typename Fn>
static void updateTelemetry(Fn updater) {
    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        updater(telemetry);
        xSemaphoreGive(telemetryMutex);
    }
}

static TelemetryData getTelemetrySnapshot();

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println();
    Serial.println("=================================");
    Serial.println("  Intelligent Rover Booting");
    Serial.println("  Board: ESP32 + Classic BT");
    Serial.println("=================================");

    telemetryMutex = xSemaphoreCreateMutex();

    if (telemetryMutex == nullptr) {
        Serial.println("[SYSTEM] FATAL: telemetry mutex allocation failed");
        while (true) {
            delay(1000);
        }
    }

    pinMode(PIN_STATUS_LED, OUTPUT);

    motor.begin();

#if ENABLE_BLUETOOTH_CONTROL
    bluetooth.begin();
#else
    Serial.println("[BT] Bluetooth control disabled");
#endif

    sensors.begin();

#if ENABLE_GPS_MODULE
    gpsManager.begin();
#else
    Serial.println("[GPS] GPS module disabled");
#endif

#if ENABLE_AI_EVENT_RECEIVER
    aiEventReceiver.begin();
#else
    Serial.println("[AI] AI event receiver disabled");
#endif

#if ENABLE_WIFI_TELEMETRY
    wifiManager.begin();
#else
    Serial.println("[WIFI] Wi-Fi telemetry disabled");
#endif

#if ENABLE_OTA_UPDATE
    otaUpdate.begin();
#else
    Serial.println("[OTA] OTA disabled");
#endif

#if ENABLE_THINGSPEAK_UPLOAD
    thingSpeakClient.begin();
#else
    Serial.println("[THINGSPEAK] ThingSpeak upload disabled");
#endif

#if ENABLE_CYD_DISPLAY
    displayBridge.begin();
#else
    Serial.println("[DISPLAY] CYD display disabled");
#endif

    lastCommandTime = millis();

#if ENABLE_BLUETOOTH_CONTROL
    xTaskCreatePinnedToCore(
        TaskBluetoothControl,
        "BT_Control",
        4096,
        nullptr,
        4,
        nullptr,
        1
    );
#endif

#if ENABLE_OTA_UPDATE
    xTaskCreatePinnedToCore(
        TaskOTAService,
        "OTA_Service",
        8192,
        nullptr,
        6,
        nullptr,
        0
    );
#endif

#if ENABLE_AI_EVENT_RECEIVER
    xTaskCreatePinnedToCore(
        TaskAIEventRead,
        "AI_Event_Read",
        4096,
        nullptr,
        3,
        nullptr,
        1
    );
#endif

    xTaskCreatePinnedToCore(
        TaskSensorRead,
        "Sensor_Read",
        4096,
        nullptr,
        2,
        nullptr,
        0
    );

#if ENABLE_GPS_MODULE
    xTaskCreatePinnedToCore(
        TaskGPSRead,
        "GPS_Read",
        4096,
        nullptr,
        2,
        nullptr,
        0
    );
#endif

    xTaskCreatePinnedToCore(
        TaskNetworkService,
        "Network_Service",
        8192,
        nullptr,
        2,
        nullptr,
        0
    );

#if ENABLE_CYD_DISPLAY
    xTaskCreatePinnedToCore(
        TaskDisplayService,
        "Display_Service",
        4096,
        nullptr,
        1,
        nullptr,
        0
    );
#endif

    xTaskCreatePinnedToCore(
        TaskStatusLED,
        "Status_LED",
        2048,
        nullptr,
        1,
        nullptr,
        0
    );

    Serial.printf("[SYSTEM] Rover ready. Default speed: %d\n", currentSpeed);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

#if ENABLE_OTA_UPDATE
void TaskOTAService(void *pvParameters) {
    while (true) {
        otaUpdate.handle();

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
#endif

void TaskBluetoothControl(void *pvParameters) {
    char speedBuf[8];
    uint8_t speedIdx = 0;
    bool inSpeedCmd = false;
    bool motorsStoppedByFailsafe = false;

    while (true) {
        while (bluetooth.available()) {
            char c = bluetooth.readCommand();

            lastCommandTime = millis();
            motorsStoppedByFailsafe = false;

            updateTelemetry([](TelemetryData &data) {
                data.bluetoothConnected = true;
            });

            if (c == '#') {
                inSpeedCmd = true;
                speedIdx = 0;
                memset(speedBuf, 0, sizeof(speedBuf));
                continue;
            }

            if (inSpeedCmd) {
                if (c == '\n' || c == '\r') {
                    inSpeedCmd = false;

                    if (speedIdx > 0) {
                        int val = atoi(speedBuf);

                        if (val >= SPEED_MIN && val <= SPEED_MAX) {
                            currentSpeed = static_cast<uint8_t>(val);
                            Serial.printf("[BT] Speed: %d\n", currentSpeed);
                        }
                    }
                } else if (isDigit(c) && speedIdx < (sizeof(speedBuf) - 1)) {
                    speedBuf[speedIdx++] = c;
                }

                continue;
            }

            if (c == '\n' || c == '\r') {
                continue;
            }

            switch (c) {
                case 'F':
                case 'f':
                    motor.forward(currentSpeed);
                    Serial.printf("[BT] Forward spd=%d\n", currentSpeed);
                    break;

                case 'B':
                case 'b':
                    motor.reverse(currentSpeed);
                    Serial.printf("[BT] Reverse spd=%d\n", currentSpeed);
                    break;

                case 'L':
                case 'l':
                    motor.turnLeft(currentSpeed);
                    Serial.printf("[BT] Left spd=%d\n", currentSpeed);
                    break;

                case 'R':
                case 'r':
                    motor.turnRight(currentSpeed);
                    Serial.printf("[BT] Right spd=%d\n", currentSpeed);
                    break;

                case 'G':
                case 'g':
                    motor.forwardLeft(currentSpeed);
                    Serial.printf("[BT] Fwd-Left spd=%d\n", currentSpeed);
                    break;

                case 'I':
                case 'i':
                    motor.forwardRight(currentSpeed);
                    Serial.printf("[BT] Fwd-Right spd=%d\n", currentSpeed);
                    break;

                case 'H':
                case 'h':
                    motor.reverseLeft(currentSpeed);
                    Serial.printf("[BT] Rev-Left spd=%d\n", currentSpeed);
                    break;

                case 'J':
                case 'j':
                    motor.reverseRight(currentSpeed);
                    Serial.printf("[BT] Rev-Right spd=%d\n", currentSpeed);
                    break;

                case 'S':
                case 's':
                case 'Q':
                case 'q':
                    motor.stop();
                    Serial.println("[BT] STOP");
                    break;

                case 'W':
                    Serial.println("[BT] Light ON");
                    break;

                case 'w':
                    Serial.println("[BT] Light OFF");
                    break;

                case 'U':
                    Serial.println("[BT] Horn ON");
                    break;

                case 'u':
                    Serial.println("[BT] Horn OFF");
                    break;

                default:
                    break;
            }
        }

        if ((millis() - lastCommandTime) > COMMAND_TIMEOUT_MS && !motorsStoppedByFailsafe) {
            motor.stop();
            motorsStoppedByFailsafe = true;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void TaskSensorRead(void *pvParameters) {
    while (true) {
        TelemetryData sensorSnapshot = getTelemetrySnapshot();

        sensors.readEnvironment(sensorSnapshot);

#if !ENABLE_ENS160_SENSOR
        sensorSnapshot.eco2Ppm = 0;
        sensorSnapshot.tvocPpb = 0;
        sensorSnapshot.airQualityIndex = 0;
        sensorSnapshot.ens160Ready = false;
#endif

#if !ENABLE_BATTERY_MONITOR
        sensorSnapshot.batteryVoltage = 0.0f;
#endif

        updateTelemetry([&sensorSnapshot](TelemetryData &data) {
            data.temperatureC = sensorSnapshot.temperatureC;
            data.humidityPercent = sensorSnapshot.humidityPercent;
            data.eco2Ppm = sensorSnapshot.eco2Ppm;
            data.tvocPpb = sensorSnapshot.tvocPpb;
            data.airQualityIndex = sensorSnapshot.airQualityIndex;
            data.ahtReady = sensorSnapshot.ahtReady;
            data.ens160Ready = sensorSnapshot.ens160Ready;
            data.mq135Raw = sensorSnapshot.mq135Raw;
            data.mq135Voltage = sensorSnapshot.mq135Voltage;
            data.batteryVoltage = sensorSnapshot.batteryVoltage;
            data.timestampMs = sensorSnapshot.timestampMs;
        });

        Serial.println("[SENSOR] -------------------");
        Serial.printf("[SENSOR] Temp: %.2f C  Humidity: %.2f %%\n",
                      sensorSnapshot.temperatureC,
                      sensorSnapshot.humidityPercent);

#if ENABLE_ENS160_SENSOR
        Serial.printf("[SENSOR] eCO2: %u ppm  TVOC: %u ppb  AQI: %u\n",
                      sensorSnapshot.eco2Ppm,
                      sensorSnapshot.tvocPpb,
                      sensorSnapshot.airQualityIndex);
#else
        Serial.println("[SENSOR] ENS160 disabled");
#endif

        Serial.printf("[SENSOR] MQ135: %u raw / %.2f V\n",
                      sensorSnapshot.mq135Raw,
                      sensorSnapshot.mq135Voltage);

#if ENABLE_BATTERY_MONITOR
        Serial.printf("[SENSOR] Battery: %.2f V\n", sensorSnapshot.batteryVoltage);
#else
        Serial.println("[SENSOR] Battery monitor disabled");
#endif

        vTaskDelay(pdMS_TO_TICKS(SENSOR_READ_INTERVAL_MS));
    }
}

#if ENABLE_GPS_MODULE
void TaskGPSRead(void *pvParameters) {
    while (true) {
        gpsManager.update();

        TelemetryData gpsSnapshot = getTelemetrySnapshot();

        gpsManager.readGPS(gpsSnapshot);

        updateTelemetry([&gpsSnapshot](TelemetryData &data) {
            data.latitude = gpsSnapshot.latitude;
            data.longitude = gpsSnapshot.longitude;
            data.gpsSpeedKmph = gpsSnapshot.gpsSpeedKmph;
            data.gpsSatellites = gpsSnapshot.gpsSatellites;
            data.gpsValid = gpsSnapshot.gpsValid;
        });

        if (gpsSnapshot.gpsValid) {
            Serial.printf("[GPS] Lat: %.6f  Lng: %.6f  Spd: %.1f km/h  Sats: %u\n",
                          gpsSnapshot.latitude,
                          gpsSnapshot.longitude,
                          gpsSnapshot.gpsSpeedKmph,
                          gpsSnapshot.gpsSatellites);
        } else {
            Serial.println("[GPS] Waiting for valid fix...");
        }

        vTaskDelay(pdMS_TO_TICKS(GPS_READ_INTERVAL_MS));
    }
}
#endif

#if ENABLE_AI_EVENT_RECEIVER
void TaskAIEventRead(void *pvParameters) {
    while (true) {
        updateTelemetry([](TelemetryData &data) {
            aiEventReceiver.update(data);
        });

        vTaskDelay(pdMS_TO_TICKS(AI_EVENT_READ_INTERVAL_MS));
    }
}
#endif

void TaskNetworkService(void *pvParameters) {
    uint32_t lastThingSpeakMs = 0;

    while (true) {
#if ENABLE_WIFI_TELEMETRY
        wifiManager.update();
#endif

        updateTelemetry([](TelemetryData &data) {
#if ENABLE_WIFI_TELEMETRY
            data.wifiConnected = wifiManager.isConnected();
            data.wifiRssi = data.wifiConnected ? WiFi.RSSI() : 0;
#else
            data.wifiConnected = false;
            data.wifiRssi = 0;
#endif

#if ENABLE_BLUETOOTH_CONTROL
            data.bluetoothConnected = bluetooth.isConnected();
#else
            data.bluetoothConnected = false;
#endif
        });

#if ENABLE_THINGSPEAK_UPLOAD
        uint32_t now = millis();

        if ((now - lastThingSpeakMs) >= THINGSPEAK_UPLOAD_INTERVAL_MS) {
            lastThingSpeakMs = now;

            TelemetryData snapshot = getTelemetrySnapshot();
            thingSpeakClient.update(snapshot);
        }
#endif

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

#if ENABLE_CYD_DISPLAY
void TaskDisplayService(void *pvParameters) {
    while (true) {
        TelemetryData snapshot = getTelemetrySnapshot();

        displayBridge.update(snapshot);

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_UPDATE_INTERVAL_MS));
    }
}
#endif

void TaskStatusLED(void *pvParameters) {
    while (true) {
        digitalWrite(PIN_STATUS_LED, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));

        digitalWrite(PIN_STATUS_LED, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static TelemetryData getTelemetrySnapshot() {
    TelemetryData snapshot;

    if (xSemaphoreTake(telemetryMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        snapshot = telemetry;
        xSemaphoreGive(telemetryMutex);
    }

    return snapshot;
}