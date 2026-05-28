#include "AIEventReceiver.h"

#include <ArduinoJson.h>
#include <cstring>

#include "config/AppConfig.h"
#include "pins/PinMap.h"

void AIEventReceiver::begin() {
    _serial.begin(AI_UART_BAUD_RATE, SERIAL_8N1, PIN_AI_UART_RX, PIN_AI_UART_TX);
    Serial.printf("[AI-UART] Receiver started RX=GPIO%d TX=GPIO%d baud=%d\n",
                  PIN_AI_UART_RX, PIN_AI_UART_TX, AI_UART_BAUD_RATE);
}

void AIEventReceiver::update(TelemetryData &data) {
    while (_serial.available()) {
        const char c = static_cast<char>(_serial.read());

        if (c == '\n' || c == '\r') {
            if (_index > 0) {
                _buffer[_index] = '\0';
                processLine(data, _buffer);
                _index = 0;
            }
            continue;
        }

        if (_index < sizeof(_buffer) - 1) {
            _buffer[_index++] = c;
        } else {
            _index = 0;
            Serial.println("[AI-UART] RX buffer overflow, packet dropped");
        }
    }
}

void AIEventReceiver::processLine(TelemetryData &data, const char *line) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, line);
    if (err) {
        Serial.print("[AI-UART] JSON parse error: ");
        Serial.println(err.c_str());
        return;
    }

    const char *type = doc["type"] | "";
    if (strcmp(type, "detection") == 0) {
        const char *label = doc["label"] | "unknown";
        data.aiDetected = true;
        strncpy(data.aiLabel, label, sizeof(data.aiLabel) - 1);
        data.aiLabel[sizeof(data.aiLabel) - 1] = '\0';
        data.aiConfidence = doc["confidence"] | 0.0f;
        data.aiX = doc["x"] | 0;
        data.aiY = doc["y"] | 0;
        data.aiW = doc["w"] | 0;
        data.aiH = doc["h"] | 0;

        Serial.printf("[AI-UART] %s %.2f box=[%d,%d,%d,%d]\n",
                      data.aiLabel, data.aiConfidence, data.aiX, data.aiY, data.aiW, data.aiH);
    } else if (strcmp(type, "heartbeat") == 0) {
        Serial.println("[AI-UART] AI node heartbeat");
    }
}
