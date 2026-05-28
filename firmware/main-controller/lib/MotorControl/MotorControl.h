#pragma once

#include <Arduino.h>

class MotorControl {
public:
    // Initialize BTS7960 motor drivers and PWM channels
    void begin();

    // Basic rover movement controls
    void forward(uint8_t speed);
    void reverse(uint8_t speed);

    void turnLeft(uint8_t speed);
    void turnRight(uint8_t speed);

    // Diagonal movement controls for Bluetooth RC apps
    void forwardLeft(uint8_t speed);
    void forwardRight(uint8_t speed);

    void reverseLeft(uint8_t speed);
    void reverseRight(uint8_t speed);

    void stop();

    // Individual motor control
    // Positive speed  -> forward
    // Negative speed  -> reverse
    // Zero            -> stop
    void setLeftMotor(int speed);
    void setRightMotor(int speed);
};