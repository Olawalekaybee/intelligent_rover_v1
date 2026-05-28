#pragma once

#include <Arduino.h>
#include "BluetoothSerial.h"

class BluetoothControl {
public:
    void begin();
    bool available();
    char readCommand();
    bool isConnected();

private:
    BluetoothSerial serialBT;
};