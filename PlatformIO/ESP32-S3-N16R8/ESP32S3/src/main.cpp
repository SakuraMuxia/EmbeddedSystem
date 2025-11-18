#include <Arduino.h>
#include <WiFi.h>
#include "camera_config.h"
#include "ws_client.h"
#include "frame_sender.h"

// WiFi 信息
const char* ssid = "wangyuan1";
const char* password = "wangyuan123$";

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\nESP32-S3 WebSocket MJPEG Streamer");
    Serial.printf("[PSRAM] Size: %d bytes\n", ESP.getPsramSize());
    Serial.printf("[PSRAM] Available: %d bytes\n", ESP.getFreePsram());
    // 摄像头
    if (!initCamera()) {
        Serial.println("Camera init failed!");
        while (1) delay(1000);
    }

    // WiFi
    WiFi.begin(ssid, password);
    Serial.print("[WiFi] Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());

    // WebSocket
    wsConnect();
}

void loop() {
    wsLoop();          // WebSocket 处理
    sendCameraFrames(); // 推送摄像头帧
}