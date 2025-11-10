#include <WiFi.h>
#include <WebServer.h>
#include <BleMouse.h>

// ======== WiFi 配置 ========
const char* ssid = "wangyuan1";
const char* password = "wangyuan123$";

// ======== BLE 鼠标对象 ========
BleMouse bleMouse("ESP32_Mouse");

// ======== HTTP 服务器 ========
WebServer server(80);

// ======== 初始化函数 ========
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 1️⃣ 连接 WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接 WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi 已连接");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.localIP());

  // 2️⃣ 启动 BLE 鼠标
  bleMouse.begin();
  Serial.println("✅ BLE 鼠标启动完成，等待手机连接...");

  // 3️⃣ 设置 HTTP 路由
  server.on("/", []() {
    server.send(200, "text/plain", "ESP32 BLE Mouse is running.\nUse /click or /swipe");
  });

  // 点击动作
  server.on("/click", []() {
    if (bleMouse.isConnected()) {
      bleMouse.click(MOUSE_LEFT);
      server.send(200, "text/plain", "Mouse click executed");
    } else {
      server.send(200, "text/plain", "BLE Mouse not connected to phone");
    }
  });

  // 滑动动作（模拟拖动）
  server.on("/swipe", []() {
    if (bleMouse.isConnected()) {
      // 模拟从上往下滑动（Y方向为负是向上，正为向下）
      for (int i = 0; i < 30; i++) {
        bleMouse.move(0, 10); // 每次移动一点
        delay(15);
      }
      server.send(200, "text/plain", "Swipe executed");
    } else {
      server.send(200, "text/plain", "BLE Mouse not connected to phone");
    }
  });

  // 启动服务器
  server.begin();
  Serial.println("✅ HTTP 服务器已启动");
}

void loop() {
  server.handleClient();
}
