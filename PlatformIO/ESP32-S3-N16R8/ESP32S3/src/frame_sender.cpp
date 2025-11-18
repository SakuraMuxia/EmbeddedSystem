#include "frame_sender.h"
#include "esp_camera.h"
#include "ws_client.h"

void sendCameraFrames()
{
    // Serial.printf("[WS] isConnected = %d\n", wsIsConnected());
    // 先快速判断连接
    if (!wsIsConnected()) {
        // 如果不连，短睡避免忙循环
        delay(10);
        return;
    }
    
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("[ESP32] fb == null");
        // 摄像头返回空，短延时再试（避免立即重试造成大量日志）
        delay(10);
        return;
    }

    // 保护性再次检查指针字段（避免奇怪的内存损坏）
    if (fb->len == 0 || fb->buf == NULL) {
        Serial.printf("[ESP32] fb invalid: len=%u buf=%p\n", fb->len, fb->buf);
        esp_camera_fb_return(fb);
        delay(10);
        return;
    }
    // Serial.printf("[ESP32] Captured frame: %u bytes\n", fb->len);
    // 推荐：分片发送，避免单次 sendBIN 阻塞或造成 TCP 局部错误
    // 发送总长度 (4 bytes, big endian)// v发送 JPEG 帧之前，先发送一个 4 字节的整数，告诉接收端这一帧 JPEG 总共有多少字节
    uint32_t frameLen = fb->len;
    uint8_t lenBuf[4] = {
        (uint8_t)(frameLen >> 24),
        (uint8_t)(frameLen >> 16),
        (uint8_t)(frameLen >> 8),
        (uint8_t)(frameLen)
    };
    wsSendFrame(lenBuf, 4);

    // 分片发送 
    const size_t CHUNK = 2048;
    size_t total = fb->len;
    const uint8_t* ptr = fb->buf;
    bool ok = true;

    for (size_t off = 0; off < total; off += CHUNK) {
        size_t chunkSize = ((off + CHUNK) < total) ? CHUNK : (total - off);
        if (!wsIsConnected()) { ok = false; Serial.println("[ESP32] connection lost"); break; }
        if (!wsSendFrame((uint8_t*)(ptr + off), chunkSize)) {
            ok = false; 
            Serial.println("[ESP32] sendBIN failed"); 
            break; 
        }
        delay(1);
    }

    // Serial.printf("[ESP32] sendBIN result: %d\n", ok ? 1 : 0);
    
    // 一定要返回 fb（且只返回一次）
    esp_camera_fb_return(fb);
    // 小睡一下，避免占满 CPU
    delay(2);
}