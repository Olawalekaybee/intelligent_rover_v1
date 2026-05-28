#include "DisplayBridge.h"

#include <Wire.h>
#include <cstring>

#include "config/AppConfig.h"
#include "protocols/DisplayPacket.h"

void DisplayBridge::begin() {
    Serial.printf("[DISPLAY] CYD I2C bridge ready at slave address 0x%02X\n", CYD_I2C_ADDRESS);
}

void DisplayBridge::update(const TelemetryData &data) {
    if ((millis() - _lastUpdateMs) < DISPLAY_UPDATE_INTERVAL_MS) return;
    _lastUpdateMs = millis();

    DisplayPacket packet;
    packet.uptimeSeconds = millis() / 1000;
    packet.temperatureCx100 = static_cast<int16_t>(data.temperatureC * 100.0f);
    packet.humidityX100 = static_cast<uint16_t>(data.humidityPercent * 100.0f);
    packet.eco2Ppm = data.eco2Ppm;
    packet.tvocPpb = data.tvocPpb;
    packet.airQualityIndex = data.airQualityIndex;
    packet.mq135Raw = data.mq135Raw;
    packet.mq135MilliVolts = static_cast<uint16_t>(data.mq135Voltage * 1000.0f);
    packet.batteryMilliVolts = static_cast<uint16_t>(data.batteryVoltage * 1000.0f);
    packet.latitudeE7 = static_cast<int32_t>(data.latitude * 10000000.0);
    packet.longitudeE7 = static_cast<int32_t>(data.longitude * 10000000.0);
    packet.gpsSatellites = data.gpsSatellites;
    packet.wifiRssi = data.wifiRssi;
    packet.aiConfidence = static_cast<uint8_t>(data.aiConfidence * 100.0f);

    if (data.wifiConnected) packet.flags |= DISPLAY_FLAG_WIFI_CONNECTED;
    if (data.bluetoothConnected) packet.flags |= DISPLAY_FLAG_BT_CONNECTED;
    if (data.gpsValid) packet.flags |= DISPLAY_FLAG_GPS_VALID;
    if (data.aiDetected) packet.flags |= DISPLAY_FLAG_AI_DETECTED;
    if (data.ahtReady) packet.flags |= DISPLAY_FLAG_AHT_READY;
    if (data.ens160Ready) packet.flags |= DISPLAY_FLAG_ENS_READY;

    strncpy(packet.aiLabel, data.aiLabel, sizeof(packet.aiLabel) - 1);

    Wire.beginTransmission(CYD_I2C_ADDRESS);
    Wire.write(reinterpret_cast<const uint8_t*>(&packet), sizeof(packet));
    const uint8_t err = Wire.endTransmission();

    if (err != 0) {
        Serial.printf("[DISPLAY] I2C update failed, error=%u\n", err);
    }
}
