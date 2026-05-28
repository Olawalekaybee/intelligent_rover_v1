# Intelligent Rover

## Distributed AI Robotics, Environmental Telemetry & Autonomous Edge Monitoring Platform

![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20XIAO%20ESP32--S3-blue)
![Framework](https://img.shields.io/badge/framework-Arduino%20%2B%20FreeRTOS-green)
![Build](https://img.shields.io/badge/build-PlatformIO-orange)
![AI](https://img.shields.io/badge/AI-Grove%20Vision%20AI%20V2-red)
![OTA](https://img.shields.io/badge/OTA-enabled-brightgreen)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

# Intelligent Rover

**Intelligent Rover** is a distributed embedded robotics platform designed for:

* AI-assisted environmental monitoring
* real-time rover mobility control
* edge AI object detection
* wireless telemetry and cloud integration
* modular embedded communication
* remote firmware management
* scalable autonomous robotics research

The system combines:

* ESP32 embedded control systems,
* XIAO ESP32-S3 AI edge processing,
* Grove Vision AI V2 object detection,
* environmental sensing,
* GPS telemetry,
* cloud telemetry upload,
* Bluetooth rover control,
* CYD telemetry dashboard display,
* and OTA firmware infrastructure.

The project is intentionally architected as a **multi-node embedded robotics system**, where independent firmware nodes communicate through UART and I2C buses to improve:

* scalability,
* modularity,
* reliability,
* fault isolation,
* maintainability,
* and distributed processing efficiency.

---

# Core Features

## Rover Mobility

* 4WD differential drive
* BTS7960 motor driver support
* Bluetooth Classic remote control
* Dynamic speed control
* Motor failsafe timeout protection
* Directional steering logic
* RTOS task-based motion management

## Environmental Monitoring

* AHT20 temperature monitoring
* AHT20 humidity monitoring
* MQ135 gas sensor integration
* ENS160 air quality monitoring
* eCO2 monitoring
* TVOC monitoring
* Air Quality Index (AQI) telemetry

## AI Vision

* Grove Vision AI V2 integration
* Person detection support
* Edge AI inference
* Real-time AI event forwarding
* AI confidence filtering
* Live video stream support through SenseCraft AI

## Telemetry & Networking

* Wi-Fi telemetry upload
* ThingSpeak cloud integration
* GPS coordinate reporting
* OTA firmware updates
* CYD telemetry dashboard
* Real-time serial diagnostics

## System Engineering

* PlatformIO build system
* FreeRTOS task scheduling
* Modular firmware architecture
* Multi-node embedded communication
* Production-grade subsystem separation
* OTA-ready deployment workflow

---

# System Architecture

```txt
                                ┌─────────────────────────────┐
                                │ Remote Operator / Dashboard │
                                └─────────────┬───────────────┘
                                              │
                                       Wi-Fi / Internet
                                              │
                   ┌──────────────────────────┼──────────────────────────┐
                   │                          │                          │
                   ▼                          ▼                          ▼
        ┌─────────────────┐      ┌─────────────────────┐      ┌─────────────────┐
        │ ESP32 Main MCU  │      │ XIAO ESP32-S3 Node │      │ CYD Display MCU │
        │─────────────────│      │─────────────────────│      │─────────────────│
        │ Motor Control   │      │ AI Processing       │      │ TFT Dashboard   │
        │ Bluetooth Ctrl  │      │ OTA                 │      │ OTA             │
        │ GPS Telemetry   │      │ Wi-Fi Stream Bridge │      │ Telemetry View  │
        │ Sensors         │      │ AI Event Filtering  │      │ Status Monitor  │
        │ ThingSpeak      │      └─────────┬───────────┘      └────────┬────────┘
        │ OTA             │                │                           │
        └────────┬────────┘                │                           │
                 │                         │                           │
                 │ UART / I2C              │ I2C                       │ I2C
                 │                         │                           │
                 ▼                         ▼                           ▼
                    ┌────────────────────────────────────┐
                    │ Grove Vision AI V2 + P5V04A Camera│
                    │------------------------------------│
                    │ Person Detection                   │
                    │ AI Inference                       │
                    │ SenseCraft AI Integration          │
                    │ Live Stream Path                   │
                    └────────────────────────────────────┘
```

---

# Distributed Node Architecture

The platform is split into independent firmware nodes.

## Main Controller Node

### Hardware

* ESP32 Dev Module
* BTS7960 Motor Drivers
* MQ135 Gas Sensor
* AHT20 Temperature/Humidity Sensor
* ENS160 Air Quality Sensor
* GPS Module
* Bluetooth Classic

### Responsibilities

* Rover mobility control
* Sensor acquisition
* GPS processing
* Bluetooth command handling
* ThingSpeak telemetry upload
* OTA firmware handling
* CYD telemetry forwarding
* AI event processing

---

## AI Camera Node

### Hardware

* XIAO ESP32-S3
* Grove Vision AI V2
* P5V04A Camera

### Responsibilities

* AI inference processing
* Person detection
* AI confidence filtering
* Live video stream handling
* AI event forwarding
* OTA firmware handling
* SenseCraft AI integration

---

## CYD Display Node

### Hardware

* CYD ESP32 Display Board
* ILI9341 TFT Display

### Responsibilities

* Real-time telemetry dashboard
* GPS display
* Sensor visualization
* Wi-Fi status display
* Bluetooth status display
* OTA firmware updates

---

# Communication Topology

| Source         | Destination        | Protocol          | Purpose                    |
| -------------- | ------------------ | ----------------- | -------------------------- |
| ESP32 Main MCU | CYD Display        | I2C               | Telemetry packets          |
| ESP32 Main MCU | Grove Vision AI V2 | I2C               | AI event acquisition       |
| XIAO ESP32-S3  | Grove Vision AI V2 | Internal I2C      | AI inference communication |
| ESP32 Main MCU | Phone App          | Bluetooth Classic | Rover movement control     |
| ESP32 Main MCU | ThingSpeak         | HTTP              | Cloud telemetry            |
| ESP32 Main MCU | Wi-Fi Router       | Wi-Fi             | OTA + networking           |
| XIAO ESP32-S3  | SenseCraft AI      | Wi-Fi / USB       | AI model management        |

---

# FreeRTOS Task Architecture

| Task              | Core | Priority | Responsibility          |
| ----------------- | ---- | -------- | ----------------------- |
| Bluetooth Control | 1    | 4        | Rover movement control  |
| Sensor Read       | 0    | 2        | Environmental telemetry |
| OTA Service       | 0    | 6        | OTA firmware updates    |
| Network Service   | 0    | 2        | Wi-Fi + ThingSpeak      |
| GPS Read          | 0    | 2        | GPS processing          |
| Display Service   | 0    | 1        | CYD updates             |
| AI Event Receiver | 1    | 3        | AI detection processing |
| Status LED        | 0    | 1        | System heartbeat        |

---

# Firmware Repository Structure

```txt
firmware/
├── main-controller/
│   ├── include/
│   ├── lib/
│   ├── src/
│   └── platformio.ini
│
├── ai-camera/
│   ├── include/
│   ├── src/
│   └── platformio.ini
│
├── cyd-display/
│   ├── include/
│   ├── src/
│   └── platformio.ini
│
├── shared/
│   ├── protocols/
│   ├── telemetry/
│   └── utilities/
│
└── docs/
    ├── wiring/
    ├── pinmaps/
    ├── diagrams/
    └── screenshots/
```

---

# Sensor Mapping

| Sensor             | Purpose                   | Interface |
| ------------------ | ------------------------- | --------- |
| AHT20              | Temperature + Humidity    | I2C       |
| ENS160             | Air Quality + eCO2 + TVOC | I2C       |
| MQ135              | Gas Detection             | ADC       |
| GPS Module         | Position Tracking         | UART      |
| Grove Vision AI V2 | AI Detection              | I2C       |

---

# ThingSpeak Field Mapping

| Data            | Field   |
| --------------- | ------- |
| Temperature     | Field 1 |
| Humidity        | Field 2 |
| eCO2            | Field 3 |
| TVOC            | Field 4 |
| MQ135 Raw       | Field 5 |
| Battery Voltage | Field 6 |
| Latitude        | Field 7 |
| Longitude       | Field 8 |

---

# Bluetooth Rover Commands

| Command | Action        |
| ------- | ------------- |
| F       | Forward       |
| B       | Reverse       |
| L       | Turn Left     |
| R       | Turn Right    |
| G       | Forward Left  |
| I       | Forward Right |
| H       | Reverse Left  |
| J       | Reverse Right |
| S / Q   | Stop          |
| #NNN    | Update speed  |

---

# AI Detection Event Format

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

# Supported AI Models

The Grove Vision AI V2 supports multiple AI inference models through SenseCraft AI.

## Recommended Models

| Model            | Purpose                |
| ---------------- | ---------------------- |
| Person Detection | Human tracking         |
| Face Detection   | Face localization      |
| COCO 80-Class    | Multi-object detection |

Default target class:

```txt
Target 0 = Person
```

---

# Development Workflow

## Build Main Controller

```bash
cd firmware/main-controller
pio run
```

## Upload Main Controller

```bash
pio run -t upload
```

## Monitor Serial Output

```bash
pio device monitor
```

---

## Build AI Camera Node

```bash
cd firmware/ai-camera
pio run
```

---

## Build CYD Display Node

```bash
cd firmware/cyd-display
pio run
```

---

# OTA Firmware Updates

After the first USB flash:

```ini
upload_protocol = espota
upload_port = intelligent-rover-main.local
```

Then upload wirelessly:

```bash
pio run -t upload
```

OTA is implemented for:

* ESP32 main controller
* XIAO ESP32-S3 AI node
* CYD display node

---

# Reliability Features

* Motor command timeout failsafe
* Brownout mitigation using buck converter
* Wi-Fi/Bluetooth coexistence optimization
* RTOS task separation
* OTA-safe architecture
* Sensor fault tolerance
* Distributed node isolation
* Modular subsystem recovery

---

# Security Practices

* Keep `Secrets.h` outside version control
* Use `Secrets.example.h` templates
* Rotate ThingSpeak keys if exposed
* Use unique OTA passwords per node
* Avoid hardcoded credentials in production

---

# Current Development Status

| Subsystem            | Status                |
| -------------------- | --------------------- |
| ESP32 rover control  | Stable                |
| Bluetooth control    | Working               |
| Motor failsafe       | Implemented           |
| MQ135 sensor         | Working               |
| AHT20 sensor         | Working               |
| ENS160 integration   | Under validation      |
| GPS framework        | Integrated            |
| ThingSpeak telemetry | Working               |
| OTA infrastructure   | Implemented           |
| CYD display bridge   | Integrated            |
| Grove Vision AI V2   | Integrated            |
| AI event framework   | Integrated            |
| Live stream support  | Active via SenseCraft |

---

# Known Issues

* OTA stability under concurrent Bluetooth traffic is under optimization.
* Some ENS160 clone boards may expose only the AHT20 address.
* CYD display I2C bridge requires dedicated synchronization tuning.

---

# Future Roadmap

## Phase 1 — Core Rover

* Bluetooth rover control
* Sensor telemetry
* OTA infrastructure
* ThingSpeak integration

## Phase 2 — AI Integration

* Person detection
* AI event routing
* CYD telemetry dashboard
* Live video stream integration

## Phase 3 — Autonomous Intelligence

* Person-follow mode
* Autonomous navigation
* Edge AI decisions
* Obstacle avoidance
* Smart environmental alerts

## Phase 4 — Advanced Robotics

* ROS2 integration
* SLAM support
* ESP-NOW mesh telemetry
* LoRa communication
* Web dashboard
* Cloud fleet management

---

# Recommended Documentation Structure

```txt
docs/
├── wiring/
├── pinmaps/
├── architecture/
├── screenshots/
├── deployment/
└── troubleshooting/
```

---

# License

MIT License

---

# Author

Designed and developed as a modular distributed robotics research platform focused on:

* embedded AI,
* distributed edge systems,
* environmental telemetry,
* autonomous robotics,
* and real-time intelligent mobility.
