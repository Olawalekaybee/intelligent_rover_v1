# Intelligent Rover
## AI Vision, Environmental Telemetry, Bluetooth Mobility & Remote Monitoring Platform

![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20XIAO%20ESP32--S3-blue)
![Framework](https://img.shields.io/badge/framework-Arduino%20%2B%20FreeRTOS-green)
![Build](https://img.shields.io/badge/build-PlatformIO-orange)
![AI](https://img.shields.io/badge/AI-Grove%20Vision%20AI%20V2-red)
![OTA](https://img.shields.io/badge/OTA-enabled-brightgreen)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## Overview

**Intelligent Rover** is a modular embedded robotics platform combining:

- manual Bluetooth rover control,
- real-time air-quality and environmental sensing,
- GPS telemetry,
- ThingSpeak IoT cloud upload,
- OTA firmware updates,
- CYD telemetry display,
- Grove Vision AI V2 based person/object detection,
- XIAO ESP32-S3 Wi-Fi bridge for AI events and live video integration.

The project is intentionally split into independent firmware nodes so that mobility, sensing, display, AI inference, and streaming can evolve independently.

---

## System Architecture

```txt
                  ┌───────────────────────────────┐
                  │ Remote Operator / Cloud Viewer │
                  └───────────────┬───────────────┘
                                  │
                           Wi-Fi / Internet
                                  │
        ┌─────────────────────────┼──────────────────────────┐
        │                         │                          │
        ▼                         ▼                          ▼
┌────────────────┐      ┌────────────────────┐      ┌─────────────────┐
│ ESP32 Rover MCU│      │ XIAO ESP32-S3 Node │      │ CYD Display Node│
│────────────────│      │────────────────────│      │─────────────────│
│ Bluetooth Ctrl │      │ Grove AI Bridge    │      │ TFT Dashboard   │
│ Motors         │      │ AI Event Forwarder │      │ I2C Telemetry   │
│ ENS160 + AHT20 │      │ Wi-Fi / OTA        │      │ Wi-Fi / OTA     │
│ MQ135          │      │ Live Stream Link   │      │ Status Display  │
│ GPS            │      └─────────┬──────────┘      └────────┬────────┘
│ ThingSpeak     │                │                          │
│ OTA            │                │ UART JSON                 │ I2C Packet
└───────┬────────┘                │                          │
        │                         ▼                          │
        │              ┌──────────────────────┐              │
        └─────────────►│ Grove Vision AI V2   │◄─────────────┘
                       │ + P5V04A Sunny Cam  │
                       │ Person Detection    │
                       └──────────────────────┘
```

---

## Firmware Workspaces

```txt
firmware/
├── main-controller/     # ESP32 rover control, sensors, GPS, IoT, OTA
├── ai-camera/           # XIAO ESP32-S3 + Grove Vision AI V2 bridge
├── cyd-display/         # CYD ESP32 TFT telemetry display + OTA
└── shared/              # Shared protocol definitions
```

---

## Main Controller — ESP32

### Responsibilities

- Classic Bluetooth rover movement control
- 4WD differential drive via BTS7960 motor drivers
- Dynamic speed control
- Motor failsafe timeout
- ENS160 + AHT20 air-quality and environmental readings
- MQ135 analog gas sensor reading
- GPS coordinate tracking
- ThingSpeak cloud telemetry
- OTA firmware upload
- UART AI-event reception from XIAO
- I2C telemetry packet output to CYD display

### Bluetooth Motion Commands

| Command | Action |
|---|---|
| F | Forward |
| B | Reverse |
| L | Turn Left |
| R | Turn Right |
| G | Forward Left |
| I | Forward Right |
| H | Reverse Left |
| J | Reverse Right |
| S / Q | Stop |
| #NNN | Speed update |

---

## AI Camera Node — XIAO ESP32-S3 + Grove Vision AI V2

### Responsibilities

- Poll Grove Vision AI V2 using SSCMA over I2C
- Read detection boxes/classes from the loaded model
- Filter detections by confidence threshold
- Send AI detections to ESP32 rover over UART as newline-delimited JSON
- Publish local Wi-Fi logs/events
- Support OTA firmware uploads
- Provide stream-link metadata for the Grove/SenseCraft live preview path

### AI Event Format

```json
{
  "type": "detection",
  "label": "person",
  "confidence": 0.92,
  "x": 122,
  "y": 122,
  "w": 102,
  "h": 74
}
```

---

## CYD Display Node

### Responsibilities

- Receive compact I2C telemetry packets from ESP32 main controller
- Display air quality, temperature, humidity, MQ135, battery, Wi-Fi, Bluetooth, GPS, and AI detection status
- Provide independent OTA update support

> Note: CYD boards usually drive the TFT display over SPI internally. In this project, **I2C is used as the communication bus between the main ESP32 and the CYD node**, not as the TFT display bus.

---

## Sensor Field Mapping

| Data | Source | ThingSpeak Field |
|---|---|---|
| Temperature | AHT20 | Field 1 |
| Humidity | AHT20 | Field 2 |
| eCO2 | ENS160 | Field 3 |
| TVOC | ENS160 | Field 4 |
| MQ135 Raw | MQ135 | Field 5 |
| Battery Voltage | ADC Divider | Field 6 |
| Latitude | GPS | Field 7 |
| Longitude | GPS | Field 8 |

---

## Build Commands

### Main Controller

```bash
cd firmware/main-controller
pio run
pio run --target upload
pio device monitor
```

### AI Camera Node

```bash
cd firmware/ai-camera
pio run
pio run --target upload
pio device monitor
```

### CYD Display Node

```bash
cd firmware/cyd-display
pio run
pio run --target upload
pio device monitor
```

---

## OTA Uploads

After the first USB flash and Wi-Fi connection, enable `espota` in each `platformio.ini` and upload wirelessly.

Example:

```bash
pio run --target upload --upload-port intelligent-rover-main.local
```

OTA is implemented for:

- ESP32 main controller,
- XIAO ESP32-S3 AI camera node,
- CYD display node.

---

## Security

- Keep `Secrets.h` out of Git.
- Use `Secrets.example.h` as a template.
- Rotate ThingSpeak keys if accidentally committed.
- Use different OTA passwords for deployed systems.

---

## Current Development Status

| Subsystem | Status |
|---|---|
| ESP32 Bluetooth rover control | Working |
| Motor failsafe | Implemented |
| ThingSpeak telemetry | Working |
| ENS160 + AHT20 module | Integrated |
| MQ135 analog sensor | Integrated |
| GPS framework | Integrated |
| OTA main controller | Implemented |
| Grove Vision AI V2 bridge | Integrated |
| XIAO OTA | Implemented |
| CYD display node | Added |
| Live video streaming | Routed through Grove Vision AI V2 / SenseCraft preview path |

---

## License

MIT License
