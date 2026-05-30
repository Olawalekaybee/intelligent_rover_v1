#pragma once

#include <Arduino.h>

// Fixed-size I2C packet sent from ESP32 main controller to CYD display node.
// Keep this below typical ESP32 I2C buffer limits.
#define DISPLAY_PACKET_MAGIC 0x524F5652UL  // 'ROVR'
#define DISPLAY_PACKET_VERSION 1

struct __attribute__((packed)) DisplayPacket {
    uint32_t magic = DISPLAY_PACKET_MAGIC;
    uint8_t version = DISPLAY_PACKET_VERSION;

    uint32_t uptimeSeconds = 0;

    int16_t temperatureCx100 = 0;
    uint16_t humidityX100 = 0;
    uint16_t eco2Ppm = 0;
    uint16_t tvocPpb = 0;
    uint8_t airQualityIndex = 0;

    uint16_t mq135Raw = 0;
    uint16_t mq135MilliVolts = 0;

    int32_t latitudeE7 = 0;
    int32_t longitudeE7 = 0;
    uint8_t gpsSatellites = 0;

    uint8_t flags = 0;
    int8_t wifiRssi = 0;

    uint8_t aiConfidence = 0;
    char aiLabel[16] = {0};
};

// flags bit map
#define DISPLAY_FLAG_WIFI_CONNECTED 0x01
#define DISPLAY_FLAG_BT_CONNECTED   0x02
#define DISPLAY_FLAG_GPS_VALID      0x04
#define DISPLAY_FLAG_AI_DETECTED    0x08
#define DISPLAY_FLAG_AHT_READY      0x10
#define DISPLAY_FLAG_ENS_READY      0x20