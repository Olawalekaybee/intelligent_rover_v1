#pragma once

#include <Arduino.h>
#include "TelemetryData.h"

class ThingSpeakClient {
public:
    void begin();
    void update(const TelemetryData &data);

private:
    uint32_t _lastUploadMs = 0;

    bool canUpload() const;
    void markUploaded();
    void uploadTelemetry(const TelemetryData &data);
};
