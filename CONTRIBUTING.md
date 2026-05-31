# Contributing to Intelligent Rover

Thank you for your interest in contributing. This project is an embedded robotics platform built on ESP32 + XIAO ESP32-S3 with FreeRTOS, PlatformIO, and edge AI. Contributions that improve firmware reliability, add hardware support, or expand the autonomous capabilities are welcome.

---

## Before You Start

- Check the [open issues](../../issues) to avoid duplicating work.
- For large changes — new hardware support, architectural changes, new firmware nodes — open an issue first to discuss the approach.
- Keep pull requests focused. One feature or fix per PR.

---

## Development Environment

**Required:**
- [PlatformIO](https://platformio.org/) — VS Code extension or CLI
- Python 3.x
- A serial terminal at 115200 baud

**Recommended:**
- VS Code with PlatformIO IDE extension
- Logic analyser for I2C debugging

---

## Project Structure

The firmware is split across three independent nodes. Changes to one node should not break the others.

```
firmware/main-controller/   — ESP32 motor, sensors, Bluetooth, ThingSpeak
firmware/ai-camera/         — XIAO ESP32-S3 OV2640 stream + Grove AI detection
firmware/cyd-display/       — CYD TFT telemetry dashboard
```

---

## Making Changes

**1. Fork and clone the repository.**

**2. Create a branch from `main`:**
```bash
git checkout -b feature/your-feature-name
```

**3. Set up Secrets.h before building:**
```bash
cp firmware/main-controller/include/config/Secrets.example.h \
   firmware/main-controller/include/config/Secrets.h
# Fill in your Wi-Fi, OTA, and ThingSpeak credentials
```

**4. Build the relevant node to verify it compiles:**
```bash
cd firmware/main-controller
pio run
```

**5. Test on hardware if possible.** Serial monitor output should show no unexpected crashes or warnings. For the main controller, verify the sensor task does not trigger `mprec.c:778 (Balloc succeeded)` — if it does, increase `STACK_SENSOR_READ` in `SystemConstants.h`.

**6. Commit with a clear message:**
```
fix(sensor): increase STACK_SENSOR_READ to prevent dtoa heap crash
feat(ai-camera): add /ai/status endpoint with detection history
docs: update I2C bridge wiring diagram
```

**7. Open a pull request** against the `main` branch. Fill in the PR template.

---

## Code Style

- Follow the existing formatting — 4-space indentation, K&R brace style.
- All tasks must be pinned to a core and given explicit stack sizes.
- Never call `Serial.printf` with float specifiers inside a mutex — use `dtostrf()` or pre-convert to integer.
- I2C work must happen **outside** the telemetry mutex. Only copy results into telemetry inside the mutex.
- Feature flags go in `AppConfig.h` using `#define ENABLE_FEATURE_NAME 0/1`. New features should default to `0`.

---

## Secrets & Security

- Never commit `Secrets.h`. It is in `.gitignore` for a reason.
- If you accidentally expose credentials, rotate them immediately and force-push to remove from history.
- See [SECURITY.md](SECURITY.md) for responsible disclosure.

---

## Submitting Issues

Use the issue templates. When reporting a crash, always include the full backtrace from the serial monitor and the firmware node it occurred in (main-controller / ai-camera / cyd-display).