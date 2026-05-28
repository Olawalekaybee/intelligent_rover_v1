#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "protocols/DisplayPacket.h"

class CYDDisplay {
public:
    void begin();
    void drawBootScreen();
    void drawTelemetry(const DisplayPacket &packet, bool packetFresh);

private:
    TFT_eSPI _tft;

    void drawHeader(bool packetFresh);
    void drawStatusLine(int y, const char *label, const String &value, uint16_t color);
};
