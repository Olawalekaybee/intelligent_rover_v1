#include "interfaces/UARTBridge.h"
#include <ArduinoJson.h>
#include "config/AppConfig.h"

// =============================================================================
// UARTBridge — sends JSON detection events to ESP32 main controller
//
// Wire:
//   XIAO pin 43 (TX) ──► ESP32 UART2 RX (GPIO13 recommended)
//   XIAO pin 44 (RX) ◄── ESP32 UART2 TX (GPIO12 recommended)
//   GND ──────────────── GND  (common ground essential)
//
// Protocol: newline-terminated JSON
//   {"type":"detection","label":"person","confidence":0.87,"x":120,"y":80,"w":64,"h":96}
//   {"type":"heartbeat","device":"Intelligent-Rover-AI-Camera"}
// =============================================================================

void UARTBridge::begin() {
    aiSerial.begin(AI_EVENT_UART_BAUD_RATE, SERIAL_8N1,
                   AI_UART_RX_PIN, AI_UART_TX_PIN);
    Serial.println("[UART] AI bridge initialized");
    Serial.printf("[UART] TX=GPIO%d  RX=GPIO%d  Baud=%d\n",
                  AI_UART_TX_PIN, AI_UART_RX_PIN, AI_EVENT_UART_BAUD_RATE);
}

void UARTBridge::sendDetection(
    const String &label,
    float confidence,
    int x, int y, int w, int h
) {
    JsonDocument doc;
    doc["type"]       = "detection";
    doc["label"]      = label;
    doc["confidence"] = round(confidence * 100.0f) / 100.0f;
    doc["x"]          = x;
    doc["y"]          = y;
    doc["w"]          = w;
    doc["h"]          = h;

    serializeJson(doc, aiSerial);
    aiSerial.println();

    Serial.print("[UART] TX: ");
    serializeJson(doc, Serial);
    Serial.println();
}

void UARTBridge::sendHeartbeat() {
    JsonDocument doc;
    doc["type"]   = "heartbeat";
    doc["device"] = DEVICE_NAME;
    doc["uptime"] = millis() / 1000;

    serializeJson(doc, aiSerial);
    aiSerial.println();

    Serial.println("[UART] Heartbeat sent");
}