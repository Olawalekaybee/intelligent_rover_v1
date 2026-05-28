#include "MotorControl.h"
#include "pins/PinMap.h"
#include "config/AppConfig.h"

// ======================================================
// LEDC PWM Channel Assignments
// ESP32 supports LEDC PWM channels
// ======================================================

#define CH_LEFT_LPWM   0
#define CH_LEFT_RPWM   1
#define CH_RIGHT_LPWM  2
#define CH_RIGHT_RPWM  3

// Diagonal steering tuning
#define DIAGONAL_INNER_SPEED_RATIO 55   // inner wheel speed = 55% of command speed
#define MOTOR_STARTUP_SETTLE_MS    150
#define MOTOR_STOP_SETTLE_MS       5

// ======================================================
// Internal Helper Functions
// ======================================================

static void enableMotorDrivers() {
    digitalWrite(PIN_LEFT_LEN, HIGH);
    digitalWrite(PIN_LEFT_REN, HIGH);
    digitalWrite(PIN_RIGHT_LEN, HIGH);
    digitalWrite(PIN_RIGHT_REN, HIGH);
}

static void disableMotorDrivers() {
    digitalWrite(PIN_LEFT_LEN, LOW);
    digitalWrite(PIN_LEFT_REN, LOW);
    digitalWrite(PIN_RIGHT_LEN, LOW);
    digitalWrite(PIN_RIGHT_REN, LOW);
}

static void writeAllPwmZero() {
    ledcWrite(CH_LEFT_LPWM, 0);
    ledcWrite(CH_LEFT_RPWM, 0);
    ledcWrite(CH_RIGHT_LPWM, 0);
    ledcWrite(CH_RIGHT_RPWM, 0);
}

static uint8_t diagonalInnerSpeed(uint8_t speed) {
    uint16_t inner = ((uint16_t)speed * DIAGONAL_INNER_SPEED_RATIO) / 100;

    if (inner > speed) {
        inner = speed;
    }

    return (uint8_t)inner;
}

// ======================================================
// Initialize Motor Driver & PWM Channels
// ======================================================

void MotorControl::begin() {
    // Configure BTS7960 enable pins
    pinMode(PIN_LEFT_LEN, OUTPUT);
    pinMode(PIN_LEFT_REN, OUTPUT);
    pinMode(PIN_RIGHT_LEN, OUTPUT);
    pinMode(PIN_RIGHT_REN, OUTPUT);

    // Keep BTS7960 disabled during boot and PWM setup
    disableMotorDrivers();

    // Configure LEDC PWM channels
    ledcSetup(CH_LEFT_LPWM, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(CH_LEFT_RPWM, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(CH_RIGHT_LPWM, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);
    ledcSetup(CH_RIGHT_RPWM, MOTOR_PWM_FREQ, MOTOR_PWM_RESOLUTION);

    // Attach PWM channels to BTS7960 PWM pins
    ledcAttachPin(PIN_LEFT_LPWM, CH_LEFT_LPWM);
    ledcAttachPin(PIN_LEFT_RPWM, CH_LEFT_RPWM);
    ledcAttachPin(PIN_RIGHT_LPWM, CH_RIGHT_LPWM);
    ledcAttachPin(PIN_RIGHT_RPWM, CH_RIGHT_RPWM);

    // Force all PWM outputs LOW before allowing motor driver enable
    writeAllPwmZero();

    delay(MOTOR_STARTUP_SETTLE_MS);

    // Keep motors disabled at startup until a movement command arrives
    disableMotorDrivers();

    Serial.println("[MOTOR] Motor controller initialized safely");
}

// ======================================================
// High-Level Motion Functions
// ======================================================

void MotorControl::forward(uint8_t speed) {
    enableMotorDrivers();

    setLeftMotor(speed);
    setRightMotor(speed);
}

void MotorControl::reverse(uint8_t speed) {
    enableMotorDrivers();

    setLeftMotor(-speed);
    setRightMotor(-speed);
}

void MotorControl::turnLeft(uint8_t speed) {
    enableMotorDrivers();

    setLeftMotor(-speed);
    setRightMotor(speed);
}

void MotorControl::turnRight(uint8_t speed) {
    enableMotorDrivers();

    setLeftMotor(speed);
    setRightMotor(-speed);
}

void MotorControl::forwardLeft(uint8_t speed) {
    enableMotorDrivers();

    uint8_t innerSpeed = diagonalInnerSpeed(speed);

    setLeftMotor(innerSpeed);
    setRightMotor(speed);
}

void MotorControl::forwardRight(uint8_t speed) {
    enableMotorDrivers();

    uint8_t innerSpeed = diagonalInnerSpeed(speed);

    setLeftMotor(speed);
    setRightMotor(innerSpeed);
}

void MotorControl::reverseLeft(uint8_t speed) {
    enableMotorDrivers();

    uint8_t innerSpeed = diagonalInnerSpeed(speed);

    setLeftMotor(-innerSpeed);
    setRightMotor(-speed);
}

void MotorControl::reverseRight(uint8_t speed) {
    enableMotorDrivers();

    uint8_t innerSpeed = diagonalInnerSpeed(speed);

    setLeftMotor(-speed);
    setRightMotor(-innerSpeed);
}

void MotorControl::stop() {
    writeAllPwmZero();

    delay(MOTOR_STOP_SETTLE_MS);

    disableMotorDrivers();
}

// ======================================================
// Low-Level Motor Control Functions
// ======================================================

void MotorControl::setLeftMotor(int speed) {
    speed = constrain(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

    if (speed != 0) {
        enableMotorDrivers();
    }

    if (speed > 0) {
        ledcWrite(CH_LEFT_LPWM, speed);
        ledcWrite(CH_LEFT_RPWM, 0);
    } else if (speed < 0) {
        ledcWrite(CH_LEFT_LPWM, 0);
        ledcWrite(CH_LEFT_RPWM, abs(speed));
    } else {
        ledcWrite(CH_LEFT_LPWM, 0);
        ledcWrite(CH_LEFT_RPWM, 0);
    }
}

void MotorControl::setRightMotor(int speed) {
    speed = constrain(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);

    if (speed != 0) {
        enableMotorDrivers();
    }

    if (speed > 0) {
        ledcWrite(CH_RIGHT_LPWM, speed);
        ledcWrite(CH_RIGHT_RPWM, 0);
    } else if (speed < 0) {
        ledcWrite(CH_RIGHT_LPWM, 0);
        ledcWrite(CH_RIGHT_RPWM, abs(speed));
    } else {
        ledcWrite(CH_RIGHT_LPWM, 0);
        ledcWrite(CH_RIGHT_RPWM, 0);
    }
}