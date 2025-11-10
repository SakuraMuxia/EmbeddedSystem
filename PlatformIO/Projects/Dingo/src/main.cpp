#include <WiFi.h>
#include <HTTPClient.h>
#include <BleMouse.h>
#include "CommandHandler.h"

const char *ssid = "wangyuan1";
const char *password = "wangyuan123$";
const char *serverUrl = "http://192.168.1.245:3000/command";

BleMouse bleMouse("ESP32_Mouse");

// ======== 辅助状态变量 ========
String lastCommand = ""; // 记录上一次执行的命令
int httpFailCount = 0;   // 连续 HTTP 失败计数

void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  Serial.println("🚀 WiFi 重新连接中...");
  WiFi.disconnect(true);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
  {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\n✅ WiFi 已重新连接");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
    httpFailCount = 0; // 重置错误计数
  }
  else
  {
    Serial.println("\n❌ WiFi 连接失败，稍后重试...");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // 连接 WiFi
  connectWiFi();
  // 2️⃣ 启动 BLE 鼠标
  bleMouse.begin();
  // ✅ 将全局指针赋值给模块
  // bleMousePtr = &bleMouse;
  // ✅ 初始化命令映射模块
  setupCommands(bleMouse); 
  Serial.println("✅ BLE 鼠标启动完成，等待手机连接...");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    delay(1000);
    return;
  }
  // 如果 BLE 未连接手机，则跳过本轮执行，减少耗电
  if (!bleMouse.isConnected())
  {
    Serial.println("⚠️ BLE 未连接手机，跳过命令轮询");
    delay(2000);
    return;
  }

  // HTTP 请求部分
  HTTPClient http;
  http.begin(serverUrl);

  int httpCode = http.GET();

  if (httpCode == 200){
    String command = http.getString();
    handleCommand(command); // ✅ 调用模块处理命令
  }

  http.end();
  delay(1000);
}