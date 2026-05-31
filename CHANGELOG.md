# Changelog

All notable changes to Intelligent Rover are documented here.

---

## [1.5.0] — 2025-05-30

### Added
- OV2640 real-time MJPEG streaming at 5–15 fps via Wi-Fi (XIAO Sense camera slot)
- Dual-camera architecture: OV2640 for live video, P5V04A for AI inference — run simultaneously
- Web viewer at `http://<ip>/` with live FPS counter, RSSI badge, heap monitor, uptime
- AI detection overlay on web viewer — person badge fades in with confidence bar on detection
- Detection history log (last 8 events with timestamp)
- `/status` JSON endpoint including detection state, confidence, label, age
- `StreamServer::notifyDetection()` static method for pipeline → viewer communication
- I2C bridge wiring guide and ASCII schematic in README

### Changed
- `TaskAIEventRead` now performs I2C work outside the telemetry mutex, eliminating up to 2-second blocking of sensor/GPS/network tasks
- `STACK_SENSOR_READ` increased from 4096 → 6144 to prevent `dtoa` heap allocation crash under BT+WiFi memory pressure
- MJPEG stream viewer replaced with periodic JPEG refresh (reliable at slow frame rates)
- `streamServer.begin()` now guarded with mutex check to prevent double-initialisation

### Fixed
- `assert failed: mprec.c:778 (Balloc succeeded)` crash in TaskSensorRead — root cause: dtoa float→string heap allocation failing under BT+WiFi fragmentation
- `StreamServer::begin()` called twice (once in setup(), once in TaskAIFrameServer) causing mutex leak
- `AIPacket` static assert mismatch — corrected expected size from 26 → 27 bytes
- `TelemetryData` missing `aiNodeOnline` field referenced by AIEventReceiver
- `AI_HEARTBEAT_TIMEOUT_MS` missing from AppConfig.h
- Double `Wire.begin()` spam eliminated via `_wireStarted` flag in AIInference
- `/capture?<timestamp>` WebServer parse error — fixed key format to `/capture?t=<timestamp>`

### Removed
- Battery monitoring subsystem fully removed from all nodes (AppConfig, PinMap, TelemetryData, SensorManager, ThingSpeakClient, DisplayBridge, DisplayPacket, main.cpp)
- `UARTBridge` — replaced by I2C bridge in ai-camera node
- `uart_bridge.cpp` stub cleared of all references to removed UART constants

---

## [1.4.0] — 2025-05-25

### Added
- ThingSpeak cloud telemetry — 7 fields, 15-second upload interval
- GPS field mapping updated: Field 6 = Latitude, Field 7 = Longitude (after battery removal)
- `[THINGSPEAK] HTTP 200 Response: NNN` log with entry number on successful upload
- Wi-Fi reconnect resilience — rover continues all local operations when offline

### Changed
- ThingSpeak field 6 was battery voltage — removed; GPS promoted from field 7/8 to field 6/7

---

## [1.3.0] — 2025-05-20

### Added
- I2C AI event bridge — replaces UART (GPIO43/44 are USB JTAG on XIAO S3, caused freeze)
- `AIEventReceiver` with silent ping (`beginTransmission/endTransmission`) before `requestFrom` to suppress Wire error spam
- XIAO I2C slave at address 0x55, Wire1 on GPIO3/4
- ESP32 I2C master on Wire1, GPIO4 (SDA) / GPIO13 (SCL)
- AIPacket 27-byte packed struct with XOR checksum
- SSCMA upgraded from v1.0.0 → GitHub main branch (fixes `isEmpty()` API)

### Fixed
- UART bridge replaced: GPIO43/44 on XIAO S3 are USB JTAG/Serial pins — using them as UART caused USB CDC freeze
- `[Wire.cpp:513] requestFrom(): i2cRead returned Error 263` spam — silent ping approach eliminates noise

---

## [1.2.0] — 2025-05-15

### Added
- Grove Vision AI V2 person detection at 89–92% confidence
- SSCMA library integration for WE2 inference chip
- AI detection events: label, confidence (0–100), bounding box (x, y, w, h)
- WiFi detection event publishing as JSON heartbeat
- Detection cooldown (1 second) to suppress duplicate events

### Changed
- AI camera node architecture: TaskDetectionPipeline on Core 1, camera/stream servers on Core 0

---

## [1.1.0] — 2025-05-10

### Added
- FreeRTOS multi-task architecture with explicit core pinning
- `TelemetryData` shared struct with mutex protection
- `RoverWiFiManager` with BT+WiFi coexistence (`WIFI_PS_MIN_MODEM` mandatory)
- OTA firmware update on all three nodes (port 3232, mDNS hostname)
- AHT20 temperature and humidity sensor
- MQ135 gas sensor with ADC voltage calculation

### Fixed
- WiFi `AUTH_EXPIRE` / `abort()` crash — root cause: `WIFI_PS_NONE` forbidden with Classic BT; fixed with `WIFI_PS_MIN_MODEM` and `disconnect(false)`

---

## [1.0.0] — 2025-05-05

### Added
- Initial tri-node firmware architecture
- ESP32 main controller with BTS7960 4WD motor control
- Bluetooth Classic SPP RC with directional commands and speed control
- Motor failsafe — auto-stop after 500 ms command silence
- XIAO ESP32-S3 AI camera node skeleton
- CYD display node skeleton
- PlatformIO build system with `huge_app.csv` partition scheme