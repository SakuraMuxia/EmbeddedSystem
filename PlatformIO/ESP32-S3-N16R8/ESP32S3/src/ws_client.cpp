#include "ws_client.h"
#include <WiFi.h>
#include <WebSocketsClient.h>

WebSocketsClient webSocket;
bool streaming = false;
// Node.js WebSocket 服务地址
const char* NODE_WS_HOST = "192.168.2.250"; // 修改为你的 Node IP
const uint16_t NODE_WS_PORT = 9001;


// 上次心跳时间
unsigned long lastPingMillis = 0;
const unsigned long PING_INTERVAL = 10000; // 10 秒发送一次 ping

// WS 事件回调
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Disconnected");
            break;
        case WStype_CONNECTED:
            Serial.println("[WS] Connected");
            // webSocket.sendTXT("ESP32"); // 标识自己为 ESP32 客户端
            // 可在此发送初始消息
            break;
        case WStype_TEXT:
            if (strncmp((char*)payload, "START", length) == 0) {
                streaming = true;
                Serial.println("[WS] Streaming started");
            } else if (strncmp((char*)payload, "STOP", length) == 0) {
                streaming = false;
                Serial.println("[WS] Streaming stopped");
            }
            break;
        case WStype_BIN:
            Serial.printf("[WS] Received binary data: %d bytes\n", length);
            break;
        default:
            break;
    }
}

bool wsConnect() {
    webSocket.begin(NODE_WS_HOST, NODE_WS_PORT, "/ESP32");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);   // 自动重连
    webSocket.enableHeartbeat(10000, 5000, 3);  
    Serial.println("[WS] WebSocket initialized");
    return true;
}

bool wsIsConnected() {
    return webSocket.isConnected();
}

bool wsSendFrame(uint8_t* data, size_t len) {
    if (wsIsConnected()) {
        return webSocket.sendBIN(data, len);
    }
    return false;
}

void wsLoop() {
    webSocket.loop();
}