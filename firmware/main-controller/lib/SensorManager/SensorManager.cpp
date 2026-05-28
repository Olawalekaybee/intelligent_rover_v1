#include "SensorManager.h"
#include "pins/PinMap.h"
#include "config/AppConfig.h"

bool SensorManager::begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

#if ENABLE_MQ135_SENSOR
    analogReadResolution(12);
    pinMode(PIN_MQ135_ADC, INPUT);
    Serial.println("[SENSOR] MQ135 analog input initialized");
#endif

#if ENABLE_BATTERY_MONITOR
    pinMode(PIN_BATTERY_ADC, INPUT);
    Serial.println("[SENSOR] Battery monitor initialized");
#endif

#if ENABLE_AHT20_SENSOR
    _ahtReady = _aht.begin(&Wire);

    if (_ahtReady) {
        Serial.println("[SENSOR] AHT20 initialized");
    } else {
        Serial.println("[SENSOR] AHT20 not detected");
    }
#else
    _ahtReady = false;
    Serial.println("[SENSOR] AHT20 disabled");
#endif

#if ENABLE_ENS160_SENSOR
    _ensReady = _ens160.begin() && _ens160.available();

    if (_ensReady) {
        _ens160.setMode(ENS160_OPMODE_STD);
        Serial.println("[SENSOR] ENS160 initialized");
    } else {
        Serial.println("[SENSOR] ENS160 not detected");
    }
#else
    _ensReady = false;
    Serial.println("[SENSOR] ENS160 disabled");
#endif

    return _ahtReady || _ensReady;
}

void SensorManager::readEnvironment(TelemetryData &data) {
    data.ahtReady = _ahtReady;
    data.ens160Ready = _ensReady;

#if ENABLE_AHT20_SENSOR
    if (_ahtReady) {
        sensors_event_t humidity;
        sensors_event_t temperature;

        _aht.getEvent(&humidity, &temperature);

        data.temperatureC = temperature.temperature;
        data.humidityPercent = humidity.relative_humidity;
    } else {
        data.temperatureC = 0.0f;
        data.humidityPercent = 0.0f;
    }
#else
    data.temperatureC = 0.0f;
    data.humidityPercent = 0.0f;
#endif

#if ENABLE_ENS160_SENSOR
    if (_ensReady) {
        if (_ahtReady) {
            _ens160.set_envdata(data.temperatureC, data.humidityPercent);
        }

        if (_ens160.measure(true)) {
            data.airQualityIndex = _ens160.getAQI();
            data.tvocPpb = _ens160.getTVOC();
            data.eco2Ppm = _ens160.geteCO2();
        }
    } else {
        data.airQualityIndex = 0;
        data.tvocPpb = 0;
        data.eco2Ppm = 0;
    }
#else
    data.airQualityIndex = 0;
    data.tvocPpb = 0;
    data.eco2Ppm = 0;
#endif

#if ENABLE_MQ135_SENSOR
    uint32_t mq135Sum = 0;

    for (uint8_t i = 0; i < MQ135_SAMPLE_COUNT; i++) {
        mq135Sum += analogRead(PIN_MQ135_ADC);
        delay(2);
    }

    const uint16_t mq135Raw = mq135Sum / MQ135_SAMPLE_COUNT;

    data.mq135Raw = mq135Raw;
    data.mq135Voltage = readMQ135Voltage(mq135Raw);

    if (millis() < MQ135_WARMUP_TIME_MS) {
        Serial.println("[MQ135] Warming up...");
    }

    if (mq135Raw <= MQ135_ADC_MIN_VALID || mq135Raw >= MQ135_ADC_MAX_VALID) {
        Serial.println("[MQ135] Warning: reading near ADC limit");
    }
#else
    data.mq135Raw = 0;
    data.mq135Voltage = 0.0f;
#endif

#if ENABLE_BATTERY_MONITOR
    data.batteryVoltage = readBatteryVoltage();
#else
    data.batteryVoltage = 0.0f;
#endif

    data.timestampMs = millis();
}

float SensorManager::readMQ135Voltage(uint16_t rawValue) const {
    return (static_cast<float>(rawValue) / ADC_MAX_COUNTS) * ADC_REFERENCE_VOLTAGE;
}

float SensorManager::readBatteryVoltage() const {
#if ENABLE_BATTERY_MONITOR
    const uint16_t raw = analogRead(PIN_BATTERY_ADC);
    const float adcVoltage = (static_cast<float>(raw) / ADC_MAX_COUNTS) * ADC_REFERENCE_VOLTAGE;

    return adcVoltage * BATTERY_DIVIDER_RATIO * BATTERY_CALIBRATION;
#else
    return 0.0f;
#endif
}