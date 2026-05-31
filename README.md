# Intelligent Rover

## Distributed AI Robotics, Environmental Telemetry & Autonomous Edge Monitoring Platform

![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20XIAO%20ESP32--S3-blue)
![Framework](https://img.shields.io/badge/framework-Arduino%20%2B%20FreeRTOS-green)
![Build](https://img.shields.io/badge/build-PlatformIO-orange)
![AI](https://img.shields.io/badge/AI-Grove%20Vision%20AI%20V2-red)
![Camera](https://img.shields.io/badge/camera-OV2640%20MJPEG-blueviolet)
![Cloud](https://img.shields.io/badge/cloud-ThingSpeak-009688)
![OTA](https://img.shields.io/badge/OTA-enabled-brightgreen)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

**Intelligent Rover** is a tri-node distributed embedded robotics platform combining real-time motor control, edge AI object detection, live video streaming, environmental telemetry, GPS tracking, and cloud integration. Each node runs independent firmware over FreeRTOS, communicating through I2C bridges to deliver a fault-tolerant, OTA-updatable autonomous rover system.

---

## Table of Contents

- [System Architecture](#system-architecture)
- [Hardware Nodes](#hardware-nodes)
- [Core Features](#core-features)
- [Communication Topology](#communication-topology)
- [FreeRTOS Task Architecture](#freertos-task-architecture)
- [Repository Structure](#repository-structure)
- [Pin & Sensor Mapping](#pin--sensor-mapping)
- [I2C Bridge Wiring](#i2c-bridge-wiring)
- [Live Camera Streaming](#live-camera-streaming)
- [ThingSpeak Cloud Telemetry](#thingspeak-cloud-telemetry)
- [Bluetooth Rover Commands](#bluetooth-rover-commands)
- [AI Detection](#ai-detection)
- [OTA Firmware Updates](#ota-firmware-updates)
- [Build & Flash Guide](#build--flash-guide)
- [Secrets & Security](#secrets--security)
- [Development Status](#development-status)
- [Known Issues](#known-issues)
- [Roadmap](#roadmap)
- [License](#license)

---

## System Architecture

```
                        ┌─────────────────────────────┐
                        │  Remote Operator / Browser  │
                        │  ThingSpeak Dashboard        │
                        └──────────┬──────────────────┘
                                   │  Wi-Fi / Internet
              ┌────────────────────┼─────────────────────┐
              │                    │                      │
              ▼                    ▼                      ▼
 ┌────────────────────┐  ┌─────────────────────┐  ┌────────────────────┐
 │  ESP32 Main MCU    │  │  XIAO ESP32-S3      │  │  CYD Display MCU   │
 │────────────────────│  │─────────────────────│  │────────────────────│
 │  BTS7960 Motors    │  │  OV2640 → MJPEG     │  │  ILI9341 TFT       │
 │  Bluetooth RC      │  │  Grove Vision AI V2 │  │  Telemetry View    │
 │  AHT20 + MQ135     │  │  SSCMA I2C (Wire0)  │  │  Status Dashboard  │
 │  GPS + ThingSpeak  │  │  I2C Slave (Wire1)  │  │  OTA Ready         │
 │  OTA (port 3232)   │  │  OTA (port 3232)    │  └────────┬───────────┘
 └─────────┬──────────┘  └──────────┬──────────┘           │
           │                        │                       │
           │ Wire1 I2C master        │ Wire1 I2C slave       │ I2C slave
           │ GPIO4 SDA / GPIO13 SCL  │ GPIO3 SDA / GPIO4 SCL │ 0x42
           │ ← 4.7 kΩ pull-ups →    │                       │
           └────────────────────────┘                       │
                    AI detection packets (0x55)             │
           ┌────────────────────────────────────────────────┘
           │ Wire0 I2C master
           │ GPIO21 SDA / GPIO22 SCL
           ▼
    ┌──────────────┐
    │  CYD @ 0x42  │
    └──────────────┘
```

---

## Hardware Nodes

### Node 1 — Main Controller

| Component | Details |
|---|---|
| MCU | ESP32 Dev Module — dual-core 240 MHz |
| Motor driver | BTS7960 dual H-bridge (×2) |
| Temperature / humidity | AHT20 — I2C 0x38 |
| Air quality | ENS160 — I2C 0x53 (optional) |
| Gas sensor | MQ135 — ADC GPIO34 |
| GPS | UART1 — GPIO16 RX / GPIO17 TX |
| Connectivity | Bluetooth Classic SPP + Wi-Fi |

### Node 2 — AI Camera

| Component | Details |
|---|---|
| MCU | Seeed XIAO ESP32-S3 Sense |
| Live camera | OV2640 (XIAO Sense camera slot) — MJPEG 5–15 fps |
| AI camera | P5V04A Sunny → Grove Vision AI V2 (Himax WE2) |
| AI protocol | SSCMA over I2C Wire0 — GPIO5 SDA / GPIO6 SCL |
| Bridge to ESP32 | I2C slave Wire1 at 0x55 — GPIO3 SDA / GPIO4 SCL |
| PSRAM | 8 MB — used for camera frame buffers |

### Node 3 — CYD Display

| Component | Details |
|---|---|
| MCU | ESP32 (CYD board) |
| Display | ILI9341 3.2" TFT 320×240 |
| Protocol | I2C slave at 0x42 |

---

## Core Features

### Rover Mobility
- 4WD differential drive with BTS7960 H-bridge motor drivers
- Bluetooth Classic RC via Dabble / Serial BT Terminal app
- Dynamic speed control via `#NNN` command (range 60–255)
- Automatic motor failsafe — stops after 500 ms of command silence
- RTOS task pinned to Core 1, priority 4

### Environmental Monitoring
- Temperature and humidity via AHT20 (I2C, ±0.3°C accuracy)
- eCO2, TVOC, and AQI via ENS160 (optional, configurable)
- Gas concentration via MQ135 ADC with warmup detection
- All data published to ThingSpeak every 15 seconds

### AI Vision
- Person detection at 89–92% confidence via Grove Vision AI V2
- Himax WE2 neural inference chip running SenseCraft AI models
- AI events forwarded to ESP32 over I2C at 200 ms intervals
- Bounding box (x, y, w, h), label, and confidence per detection
- AI overlay on web viewer — detection badge fades in on person detected

### Live Camera Streaming
- OV2640 streams real-time MJPEG at 5–15 fps over Wi-Fi
- Viewer at `http://<ip>/` with live FPS counter and detection overlay
- Raw MJPEG feed at `http://<ip>:81/stream`
- OV2640 uses ESP32-S3 DVP camera peripheral — fully independent of AI I2C

### Cloud & Networking
- ThingSpeak upload every 15 seconds — 7 data fields
- GPS coordinate reporting via TinyGPSPlus (NMEA UART)
- OTA firmware updates on all three nodes
- Wi-Fi offline resilience — rover continues all local ops without internet
- Auto-reconnect with 3-attempt retry on disconnect

---

## Communication Topology

| Source | Destination | Protocol | Purpose |
|---|---|---|---|
| XIAO ESP32-S3 | Grove Vision AI V2 | I2C Wire0 GPIO5/6 | SSCMA AI inference |
| XIAO ESP32-S3 | ESP32 Main | I2C Wire1 slave 0x55 | AI detection events |
| ESP32 Main | CYD Display | I2C master 0x42 | Telemetry packets |
| Mobile app | ESP32 Main | Bluetooth Classic SPP | Rover RC commands |
| ESP32 Main | ThingSpeak | HTTP GET | Cloud telemetry |
| All nodes | Wi-Fi | OTA port 3232 | Firmware updates |

---

## FreeRTOS Task Architecture

### Main Controller

| Task | Core | Priority | Stack | Responsibility |
|---|---|---|---|---|
| Bluetooth Control | 1 | 4 | 4096 | RC commands + motor failsafe |
| AI Event Receiver | 1 | 3 | 4096 | I2C master polling XIAO |
| Sensor Read | 0 | 2 | **6144** | AHT20 + MQ135 (increased for dtoa heap) |
| GPS Read | 0 | 2 | 4096 | TinyGPSPlus UART processing |
| Network Service | 0 | 2 | 8192 | Wi-Fi + ThingSpeak upload |
| OTA Service | 0 | 6 | 8192 | OTA firmware handler |
| Display Service | 0 | 1 | 4096 | CYD I2C bridge |
| Status LED | 0 | 1 | 2048 | Heartbeat blink |

### AI Camera Node

| Task | Core | Priority | Stack | Responsibility |
|---|---|---|---|---|
| Detection Pipeline | 1 | 3 | 8192 | SSCMA inference + I2C bridge |
| Camera Server | 0 | 2 | 8192 | OV2640 MJPEG on port 81 |
| AI Frame Server | 0 | 2 | 4096 | StreamServer — viewer + status |
| WiFi Bridge | 0 | 2 | 4096 | Wi-Fi management + OTA |
| Status LED | 0 | 1 | 2048 | Heartbeat blink |

---

## Repository Structure

```
intelligent_rover_v1/
├── firmware/
│   ├── main-controller/
│   │   ├── include/
│   │   │   ├── config/          AppConfig.h  NetworkConfig.h  Secrets.h
│   │   │   ├── constants/       SystemConstants.h
│   │   │   ├── pins/            PinMap.h
│   │   │   └── protocols/       DisplayPacket.h
│   │   ├── lib/
│   │   │   ├── AIEventReceiver/    I2C master — polls XIAO at 0x55
│   │   │   ├── BluetoothControl/   Classic SPP RC
│   │   │   ├── DisplayBridge/      I2C master — CYD display at 0x42
│   │   │   ├── GPSManager/         TinyGPSPlus NMEA wrapper
│   │   │   ├── MotorControl/       BTS7960 PWM 4WD
│   │   │   ├── OTAUpdate/          ArduinoOTA wrapper
│   │   │   ├── RoverWiFiManager/   Wi-Fi + BT coexistence
│   │   │   ├── SensorManager/      AHT20 + ENS160 + MQ135
│   │   │   ├── TelemetryManager/   Shared TelemetryData struct
│   │   │   └── ThingSpeakClient/   HTTP upload
│   │   ├── src/
│   │   │   └── main.cpp
│   │   └── platformio.ini
│   │
│   ├── ai-camera/
│   │   ├── include/
│   │   │   ├── config/          AppConfig.h  Secrets.h
│   │   │   └── interfaces/      AIInference.h  DetectionPipeline.h  WiFiBridge.h
│   │   ├── lib/
│   │   │   ├── CameraStream/    OV2640 MJPEG — esp_camera DVP
│   │   │   ├── I2CBridge/       I2C slave 0x55 — AIPacket 27 bytes
│   │   │   └── StreamServer/    HTTP — viewer / status / AI frames
│   │   ├── src/
│   │   │   ├── main.cpp
│   │   │   ├── ai_inference.cpp
│   │   │   ├── detection_pipeline.cpp
│   │   │   └── wifi_bridge.cpp
│   │   └── platformio.ini
│   │
│   └── cyd-display/
│       ├── include/
│       ├── lib/
│       │   └── CYDDisplay/
│       ├── src/
│       │   └── main.cpp
│       └── platformio.ini
│
├── docs/
│   ├── wiring/
│   ├── pinmaps/
│   ├── diagrams/
│   └── screenshots/
│
├── CHANGELOG.md
└── README.md
```

---

## Pin & Sensor Mapping

### Main Controller (ESP32)

| Pin | Function | Notes |
|---|---|---|
| GPIO21 / GPIO22 | I2C Wire0 SDA / SCL | AHT20, ENS160, CYD display |
| GPIO4 / GPIO13 | I2C Wire1 SDA / SCL | XIAO AI bridge master |
| GPIO16 / GPIO17 | GPS UART RX / TX | TinyGPSPlus |
| GPIO34 | MQ135 ADC | Input-only, 12-bit |
| GPIO25/26/33/32 | Left motor BTS7960 | RPWM / LPWM |
| GPIO27/14/18/19 | Right motor BTS7960 | RPWM / LPWM |
| GPIO2 | Status LED | Active high |

### AI Camera Node (XIAO ESP32-S3 Sense)

| Pin | Function | Notes |
|---|---|---|
| GPIO5 / GPIO6 | I2C Wire0 SDA / SCL | Grove Vision AI V2 SSCMA |
| GPIO3 / GPIO4 | I2C Wire1 SDA / SCL | I2C slave to ESP32 (0x55) |
| GPIO10/40/39/13/38/47/48/11/12/14/16/18/17/15 | OV2640 DVP | XIAO Sense camera slot |

---

## I2C Bridge Wiring

Connects the XIAO ESP32-S3 (slave) to the ESP32 main controller (master) for AI event forwarding.

```
XIAO ESP32-S3              4.7 kΩ          ESP32 Main Controller
─────────────              ───────          ─────────────────────
D2  (GPIO3)  ────────────┬──/\/\/──── 3.3V
                         └──────────────── GPIO4   (Wire1 SDA)

D3  (GPIO4)  ────────────┬──/\/\/──── 3.3V
                         └──────────────── GPIO13  (Wire1 SCL)

GND          ─────────────────────────── GND
```

> **Important:** Pull-up resistors are mandatory. Use 4.7 kΩ to 3.3 V only — never 5 V. Keep wire length under 20 cm. Twist SDA/SCL together to reduce motor noise interference.

**Expected serial output on ESP32 after wiring:**
```
[AI-I2C] XIAO node online
[AI-I2C] person  conf=0.92  box=[122,117,224,196]
```

---

## Live Camera Streaming

Two cameras operate simultaneously on the AI Camera node.

| Endpoint | Source | URL | Frame rate |
|---|---|---|---|
| Live viewer | HTML page | `http://<ip>/` | — |
| MJPEG stream | OV2640 | `http://<ip>:81/stream` | 5–15 fps |
| Snapshot | OV2640 | `http://<ip>:81/capture` | single frame |
| AI overlay | Grove Vision AI V2 | `http://<ip>/ai/capture` | ~0.5 fps |
| Status JSON | System | `http://<ip>/status` | — |

The web viewer includes a live FPS counter, RSSI indicator, heap monitor, uptime display, AI detection overlay badge, confidence bar, and detection history log — all updating every second via `/status` polling.

---

## ThingSpeak Cloud Telemetry

Channel updates every 15 seconds when Wi-Fi is connected.

| Field | Data | Source |
|---|---|---|
| Field 1 | Temperature (°C) | AHT20 |
| Field 2 | Humidity (%) | AHT20 |
| Field 3 | eCO2 (ppm) | ENS160 |
| Field 4 | TVOC (ppb) | ENS160 |
| Field 5 | MQ135 raw ADC | GPIO34 |
| Field 6 | GPS Latitude | GPS module |
| Field 7 | GPS Longitude | GPS module |

The rover continues all local operations when offline and resumes uploading automatically on reconnection.

---

## Bluetooth Rover Commands

Send commands from any SPP terminal app (Dabble, Serial Bluetooth Terminal).

| Command | Action | Notes |
|---|---|---|
| `F` | Forward | — |
| `B` | Reverse | — |
| `L` | Turn left | — |
| `R` | Turn right | — |
| `G` | Forward-left diagonal | — |
| `I` | Forward-right diagonal | — |
| `H` | Reverse-left diagonal | — |
| `J` | Reverse-right diagonal | — |
| `S` / `Q` | Stop | — |
| `#NNN` | Set speed | e.g. `#180` — range 60–255 |
| `W` / `w` | Lights on / off | — |
| `U` / `u` | Horn on / off | — |

**Failsafe:** motors stop automatically after 500 ms of command silence.

---

## AI Detection

### I2C Packet Protocol (XIAO → ESP32, 27 bytes packed)

```cpp
#pragma pack(push, 1)
struct AIPacket {
    uint8_t  detected;      // 1 = valid detection
    uint8_t  confidence;    // 0–100
    char     label[12];     // null-terminated class label
    int16_t  x, y, w, h;   // bounding box pixels
    uint32_t uptime;        // XIAO uptime in seconds
    uint8_t  checksum;      // XOR of bytes 0–25
};  // 27 bytes
#pragma pack(pop)
```

### WiFi Detection Event (JSON)

```json
{
  "device": "Intelligent-Rover-AI-Camera",
  "type": "detection",
  "label": "person",
  "confidence": 0.92,
  "x": 122,
  "y": 117,
  "w": 224,
  "h": 196,
  "uptime": 1188
}
```

### Supported Models via SenseCraft AI

| Model | Purpose | Target class |
|---|---|---|
| Person detection | Human tracking | 0 = person |
| Face detection | Face localization | — |
| COCO 80-class | Multi-object detection | 0–79 |

Flash models at [sensecraft.seeed.cc/ai](https://sensecraft.seeed.cc/ai) using Chrome browser connected via USB.

---

## OTA Firmware Updates

All three nodes support wireless OTA after the first USB flash.

**Main Controller**
```ini
upload_protocol = espota
upload_port     = intelligent-rover-main.local
```

**AI Camera**
```ini
upload_protocol = espota
upload_port     = rover-camera.local
```

Then upload wirelessly:
```bash
pio run -t upload
```

OTA passwords are defined in each node's `Secrets.h`.

---

## Build & Flash Guide

### Prerequisites

- [PlatformIO](https://platformio.org/) — VS Code extension or CLI
- Python 3.x
- USB cables for initial flash

### Secrets setup

```bash
cp firmware/main-controller/include/config/Secrets.example.h \
   firmware/main-controller/include/config/Secrets.h
```

```cpp
// Secrets.h — never commit this file
#define WIFI_SSID            "your_ssid"
#define WIFI_PASSWORD        "your_password"
#define OTA_PASSWORD         "your_ota_password"
#define OTA_HOSTNAME         "intelligent-rover-main"
#define THINGSPEAK_API_KEY   "your_write_key"
```

### Main Controller

```bash
cd firmware/main-controller
pio run                       # compile
pio run -t upload             # flash via USB (/dev/ttyUSB0)
pio device monitor            # serial output at 115200
```

### AI Camera Node

```bash
cd firmware/ai-camera
pio run -e xiao-esp32s3-ai-camera
pio run -e xiao-esp32s3-ai-camera -t upload   # /dev/ttyACM0
pio device monitor --port /dev/ttyACM0
```

### CYD Display

```bash
cd firmware/cyd-display
pio run
pio run -t upload
```

---

## Secrets & Security

- `Secrets.h` is excluded from version control via `.gitignore` — **never commit it**
- Use `Secrets.example.h` as an onboarding template
- Rotate ThingSpeak write keys if a repository is made public
- Use unique OTA passwords per node
- The XIAO slave address (0x55) and CYD address (0x42) avoid conflicts with all sensor addresses on the same bus

Add to `.gitignore` if not present:
```
include/config/Secrets.h
```

---

## Development Status

| Subsystem | Status | Notes |
|---|---|---|
| ESP32 boot + RTOS | ✅ Stable | 8 tasks, correct core pinning |
| Bluetooth RC | ✅ Working | Failsafe confirmed |
| AHT20 sensor | ✅ Working | ±0.3°C, stable readings |
| MQ135 gas sensor | ✅ Working | Requires 3–5 min warmup |
| ENS160 air quality | ⚙️ Optional | Enable via `ENABLE_ENS160_SENSOR=1` |
| GPS framework | ✅ Running | Fix requires outdoor clear sky |
| ThingSpeak upload | ✅ Working | HTTP 200, 15 s interval confirmed |
| OTA — all nodes | ✅ Implemented | Port 3232, mDNS hostname |
| CYD display bridge | ⚙️ Optional | Enable via `ENABLE_CYD_DISPLAY=1` |
| Grove Vision AI V2 | ✅ Working | 89–92% confidence person detection |
| OV2640 live stream | ✅ Working | MJPEG 5–15 fps via Wi-Fi |
| I2C AI event bridge | ⚙️ Wiring | Requires XIAO↔ESP32 jumper wires |
| Wi-Fi offline resilience | ✅ Confirmed | Auto-reconnect, 3-attempt retry |
| Web viewer | ✅ Working | Detection overlay, stats, detection log |

---

## Known Issues

- **MQ135 warmup**: the MQ135 heater requires 3–5 minutes after power-on before readings stabilise. Low ADC values on cold start are expected.
- **OTA under BT load**: OTA stability under concurrent Bluetooth traffic is under optimisation. Recommend stopping BT traffic before initiating OTA.
- **ENS160 clones**: some ENS160 clone modules only expose the AHT20 I2C address. Verify with an I2C scanner before enabling `ENABLE_ENS160_SENSOR`.
- **Grove V2 image fps**: SSCMA image transfer over I2C at 400 kHz is limited to ~0.5–1 fps. The OV2640 is used for real-time video.
- **Mobile hotspot Wi-Fi**: MJPEG stream fps is limited by hotspot throughput (~5 fps at −70 dBm). A dedicated router improves this to 10–15 fps.

---

## Roadmap

### Phase 1 — Core Rover ✅ Complete
Bluetooth control, motor failsafe, sensor telemetry, OTA infrastructure, ThingSpeak integration.

### Phase 2 — AI Integration ✅ Complete
Person detection at 89–92%, AI event routing over I2C, OV2640 live MJPEG streaming, web viewer with detection overlay.

### Phase 3 — Autonomous Intelligence
Person-follow mode, autonomous navigation with obstacle avoidance, edge AI decision engine, smart environmental alerts, GPS waypoint tracking.

### Phase 4 — Advanced Robotics
ROS2 integration, SLAM mapping, ESP-NOW mesh telemetry between rover nodes, LoRa long-range communication, cloud fleet management dashboard, H.264 video stream.

---

## License

MIT License — see `LICENSE` for details.

---

## Author

Designed and developed as a modular distributed robotics research platform demonstrating the integration of:

- embedded AI inference on constrained hardware
- distributed edge computing across multiple MCU nodes
- real-time environmental telemetry and cloud publishing
- autonomous robotics control with safety failsafes
- production-grade OTA firmware deployment workflows