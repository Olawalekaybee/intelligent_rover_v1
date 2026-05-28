#pragma once

#include <Arduino.h>
#include <WiFi.h>

class RoverWiFiManager {
public:
    void begin();
    void update();

    bool isConnected();
    IPAddress getIP();
    int32_t getRSSI();

private:
    uint32_t _lastReconnectAttempt = 0;

    void reconnect();
};