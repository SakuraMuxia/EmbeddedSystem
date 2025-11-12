#include <WiFi.h>
#include <HTTPClient.h>

#include <BleComboKeyboard.h>
#include <BleComboMouse.h>

#include "HttpReporter.h"
#include "CommandHandler.h"

const char *ssid = "wangyuan1";
const char *password = "wangyuan123$";
// 接收指令的地址
const char *serverUrl = "http://192.168.1.245:3000/command";
// 发送日志的地址
const char *logServerUrl = "http://192.168.1.245:3000/log";

BleComboKeyboard Keyboard("MyESP32_Combo");
BleComboMouse Mouse(&Keyboard);

// ======== 辅助状态变量 ========
String lastCommand = ""; // 记录上一次执行的命令
int httpFailCount = 0;   // 连续 HTTP 失败计数

unsigned long lastBleCheck = 0;
const unsigned long BLE_CHECK_INTERVAL = 2000; // 2秒

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
  Keyboard.begin(); // ✅ 初始化 BLE + 名称
  Mouse.begin();    // ✅ 共享同一 BLE HID profile
  Serial.println("✅ BLE Combo (Mouse + Keyboard) 启动完成，设备名：MyESP32_Combo");
  // ✅ 将全局指针赋值给模块
  // bleMousePtr = &bleMouse;
  // ✅ 初始化命令映射模块
  setupCommands(); // 初始化命令模块
  // ✅ 初始化日志上报模块
  initHttpReporter(logServerUrl);
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
  unsigned long now = millis();
  // 仅每隔 BLE_CHECK_INTERVAL 检查一次 BLE 连接状态
  if (now - lastBleCheck >= BLE_CHECK_INTERVAL)
  {
    lastBleCheck = now;

    if (!Keyboard.isConnected())
    { // 使用 Keyboard 或 Mouse 都行
      Serial.println("⚠️ BLE 未连接手机");
      return; // 本轮直接跳过，不阻塞
    }
  }

  // HTTP 请求部分
  HTTPClient http;
  http.begin(serverUrl);

  int httpCode = http.GET();

  if (httpCode == 200)
  {
    String command = http.getString();
    handleCommand(command); // ✅ 调用模块处理命令
  }

  http.end();
  delay(1000);
}