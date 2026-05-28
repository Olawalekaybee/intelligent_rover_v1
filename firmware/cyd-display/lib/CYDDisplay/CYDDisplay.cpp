#include "CYDDisplay.h"

void CYDDisplay::begin() {
    _tft.init();
    _tft.setRotation(1);
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextFont(2);
}

void CYDDisplay::drawBootScreen() {
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(TFT_CYAN, TFT_BLACK);
    _tft.drawString("Intelligent Rover", 10, 10, 4);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString("CYD Telemetry Display", 10, 45, 2);
    _tft.drawString("Waiting for ESP32 main...", 10, 75, 2);
}

void CYDDisplay::drawTelemetry(const DisplayPacket &p, bool packetFresh) {
    _tft.fillScreen(TFT_BLACK);
    drawHeader(packetFresh);

    const bool wifi = p.flags & DISPLAY_FLAG_WIFI_CONNECTED;
    const bool bt = p.flags & DISPLAY_FLAG_BT_CONNECTED;
    const bool gps = p.flags & DISPLAY_FLAG_GPS_VALID;
    const bool ai = p.flags & DISPLAY_FLAG_AI_DETECTED;

    drawStatusLine(35, "WiFi", wifi ? "ON " + String(p.wifiRssi) + " dBm" : "OFF", wifi ? TFT_GREEN : TFT_RED);
    drawStatusLine(55, "Bluetooth", bt ? "CONNECTED" : "WAITING", bt ? TFT_GREEN : TFT_ORANGE);
    drawStatusLine(75, "GPS", gps ? "FIX sats=" + String(p.gpsSatellites) : "NO FIX", gps ? TFT_GREEN : TFT_ORANGE);

    _tft.setTextColor(TFT_CYAN, TFT_BLACK);
    _tft.drawString("Environment", 10, 105, 2);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString("Temp: " + String(p.temperatureCx100 / 100.0f, 2) + " C", 10, 128, 2);
    _tft.drawString("Hum : " + String(p.humidityX100 / 100.0f, 2) + " %", 10, 148, 2);
    _tft.drawString("eCO2: " + String(p.eco2Ppm) + " ppm", 10, 168, 2);
    _tft.drawString("TVOC: " + String(p.tvocPpb) + " ppb", 10, 188, 2);
    _tft.drawString("AQI : " + String(p.airQualityIndex), 10, 208, 2);

    _tft.drawString("MQ135: " + String(p.mq135Raw) + " / " + String(p.mq135MilliVolts) + "mV", 160, 128, 2);
    _tft.drawString("Batt : " + String(p.batteryMilliVolts / 1000.0f, 2) + " V", 160, 148, 2);

    if (gps) {
        _tft.drawString("Lat: " + String(p.latitudeE7 / 10000000.0, 6), 160, 178, 2);
        _tft.drawString("Lng: " + String(p.longitudeE7 / 10000000.0, 6), 160, 198, 2);
    }

    _tft.setTextColor(ai ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
    _tft.drawString("AI: " + String(ai ? p.aiLabel : "none") + " " + String(p.aiConfidence) + "%", 10, 232, 2);
}

void CYDDisplay::drawHeader(bool packetFresh) {
    _tft.setTextColor(packetFresh ? TFT_GREEN : TFT_RED, TFT_BLACK);
    _tft.drawString(packetFresh ? "ROVER ONLINE" : "NO DATA", 10, 8, 4);
}

void CYDDisplay::drawStatusLine(int y, const char *label, const String &value, uint16_t color) {
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.drawString(String(label) + ":", 10, y, 2);
    _tft.setTextColor(color, TFT_BLACK);
    _tft.drawString(value, 95, y, 2);
}
