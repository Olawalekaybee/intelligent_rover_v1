#include "GPSManager.h"
#include "pins/PinMap.h"
#include "config/AppConfig.h"

void GPSManager::begin() {
#if ENABLE_GPS_MODULE
    gpsSerial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.println("[GPS] GPS serial started");
#else
    Serial.println("[GPS] GPS module disabled");
#endif
}

void GPSManager::update() {
#if ENABLE_GPS_MODULE
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }
#endif
}

void GPSManager::readGPS(TelemetryData &data) {
#if ENABLE_GPS_MODULE
    data.gpsValid = gps.location.isValid();

    if (data.gpsValid) {
        data.latitude = gps.location.lat();
        data.longitude = gps.location.lng();
    }

    if (gps.speed.isValid()) {
        data.gpsSpeedKmph = gps.speed.kmph();
    }

    if (gps.satellites.isValid()) {
        data.gpsSatellites = gps.satellites.value();
    }
#else
    data.gpsValid = false;
    data.latitude = 0.0;
    data.longitude = 0.0;
    data.gpsSpeedKmph = 0.0;
    data.gpsSatellites = 0;
#endif
}