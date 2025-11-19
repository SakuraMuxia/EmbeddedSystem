#include "camera_config.h"
#include <Arduino.h>
#include "board_config.h"

bool initCamera()
{
    Serial.println("[Camera] Initializing...");

    if (!psramFound())
    {
        Serial.println("⚠️ PSRAM not found! Using DRAM.");
    }

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    // 直接使用官方宏
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;

    config.xclk_freq_hz = 10000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // 固定分辨率，最安全
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 12;

    // 单缓冲
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("[Camera] Init failed 0x%x\n", err);
        return false;
    }

    Serial.println("[Camera] Ready.");
    // ========== ⬇⬇⬇ 在这里设置摄像头 FPS（正确位置）⬇⬇⬇ ==========
    sensor_t *s = esp_camera_sensor_get();
    if (s)

    {   
        s->set_reg(s, 0x3035, 0xFF, 0x11);
        s->set_reg(s, 0x3036, 0xFF, 0x46);
        s->set_reg(s, 0x3037, 0xFF, 0x03);
        s->set_reg(s, 0x3824, 0xFF, 0x02);
        // s->set_reg(s, 0x3035, 0xFF, 0x11); // PRE_PLL：略降低预分频
        // s->set_reg(s, 0x3036, 0xFF, 0xF0); // PLL_MULTIPLIER：适度增加 VCO 输出
        // s->set_reg(s, 0x3037, 0xFF, 0x00); // PLL_DIVIDER：不分频

        // // 帧间延迟寄存器：减小值，提高 FPS
        // s->set_reg(s, 0x3824, 0xFF, 0x02); // 从原来的 0x04 → 0x02，大约 20 FPS
    }
    return true;
}