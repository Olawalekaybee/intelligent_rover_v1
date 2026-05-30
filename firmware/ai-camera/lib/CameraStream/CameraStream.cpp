#include "CameraStream.h"
#include "config/AppConfig.h"

// =============================================================================
// OV2640 pin definitions — XIAO ESP32-S3 Sense
// =============================================================================
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    10
#define CAM_PIN_SIOD    40   // camera I2C SDA (NOT the same as Grove I2C GPIO5)
#define CAM_PIN_SIOC    39   // camera I2C SCL (NOT GPIO6)
#define CAM_PIN_D7      48
#define CAM_PIN_D6      11
#define CAM_PIN_D5      12
#define CAM_PIN_D4      14
#define CAM_PIN_D3      16
#define CAM_PIN_D2      18
#define CAM_PIN_D1      17
#define CAM_PIN_D0      15
#define CAM_PIN_VSYNC   38
#define CAM_PIN_HREF    47
#define CAM_PIN_PCLK    13

static const char MJPEG_BOUNDARY[] = "frame";

// =============================================================================
bool CameraStream::begin() {
    camera_config_t cfg = {};
    cfg.pin_pwdn     = CAM_PIN_PWDN;
    cfg.pin_reset    = CAM_PIN_RESET;
    cfg.pin_xclk     = CAM_PIN_XCLK;
    cfg.pin_sccb_sda = CAM_PIN_SIOD;
    cfg.pin_sccb_scl = CAM_PIN_SIOC;
    cfg.pin_d7       = CAM_PIN_D7;
    cfg.pin_d6       = CAM_PIN_D6;
    cfg.pin_d5       = CAM_PIN_D5;
    cfg.pin_d4       = CAM_PIN_D4;
    cfg.pin_d3       = CAM_PIN_D3;
    cfg.pin_d2       = CAM_PIN_D2;
    cfg.pin_d1       = CAM_PIN_D1;
    cfg.pin_d0       = CAM_PIN_D0;
    cfg.pin_vsync    = CAM_PIN_VSYNC;
    cfg.pin_href     = CAM_PIN_HREF;
    cfg.pin_pclk     = CAM_PIN_PCLK;

    cfg.xclk_freq_hz = 20000000;          // 20 MHz
    cfg.ledc_timer   = LEDC_TIMER_0;
    cfg.ledc_channel = LEDC_CHANNEL_0;

    cfg.pixel_format = PIXFORMAT_JPEG;
    cfg.frame_size   = FRAMESIZE_QVGA;    // 320x240 — good balance of fps + quality
    cfg.jpeg_quality = 30;                // 30 = good balance of quality vs file size vs fps
    cfg.fb_count     = 3;                 // triple buffer for smoother streaming
    cfg.grab_mode    = CAMERA_GRAB_LATEST; // always get the newest frame

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("[CAM] OV2640 init failed: 0x%x\n", err);
        Serial.println("[CAM] Check: is camera module connected to XIAO Sense slot?");
        _ready = false;
        return false;
    }

    // Tune the sensor
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 1);    // -2 to 2
        s->set_saturation(s, 0);    // -2 to 2
        s->set_sharpness(s, 1);     // 0 to 2
        s->set_denoise(s, 1);
        s->set_whitebal(s, 1);      // auto white balance
        s->set_awb_gain(s, 1);
        s->set_exposure_ctrl(s, 1); // auto exposure
        s->set_aec2(s, 1);
        s->set_gain_ctrl(s, 1);     // auto gain
        s->set_hmirror(s, 0);
        s->set_vflip(s, 0);
    }

    _ready = true;
    Serial.println("[CAM] OV2640 ready — QVGA 320x240 JPEG");
    return true;
}

// =============================================================================
// streamToClient — pushes MJPEG frames until client disconnects
// Call from a dedicated RTOS task
// =============================================================================
void CameraStream::streamToClient(WiFiClient &client) {
    client.print(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n\r\n"
    );

    uint32_t frameCount = 0;
    uint32_t startMs    = millis();

    while (client.connected()) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        client.printf(
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %u\r\n\r\n",
            fb->len
        );
        client.write(fb->buf, fb->len);
        client.print("\r\n");

        esp_camera_fb_return(fb);
        frameCount++;

        // Log fps every 50 frames
        if (frameCount % 50 == 0) {
            float fps = (float)frameCount / ((millis() - startMs) / 1000.0f);
            Serial.printf("[CAM] Stream: %u frames  %.1f fps\n", frameCount, fps);
        }
    }

    Serial.printf("[CAM] Client disconnected after %u frames\n", frameCount);
}

// =============================================================================
// serveSingleFrame — returns one JPEG as HTTP response
// =============================================================================
void CameraStream::serveSingleFrame(WiFiClient &client) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        client.print("HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\n\r\n");
        return;
    }

    client.printf(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/jpeg\r\n"
        "Content-Length: %u\r\n"
        "Content-Disposition: inline; filename=rover.jpg\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: no-store\r\n\r\n",
        fb->len
    );
    client.write(fb->buf, fb->len);
    esp_camera_fb_return(fb);
}