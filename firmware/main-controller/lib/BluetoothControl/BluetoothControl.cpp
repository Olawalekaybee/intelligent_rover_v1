#include "BluetoothControl.h"

void BluetoothControl::begin() {
    serialBT.begin("IntelligentRover");

    Serial.println("[BT] Classic Bluetooth Started");
}

bool BluetoothControl::available() {
    return serialBT.available();
}

char BluetoothControl::readCommand() {
    return serialBT.read();
}

bool BluetoothControl::isConnected() {
    return serialBT.hasClient();
}