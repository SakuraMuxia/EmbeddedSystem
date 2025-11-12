#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <BleComboKeyboard.h>
#include <BleComboMouse.h>
#include "CommandHandler.h"

using namespace websockets;

// ======== 配置信息 ========
const char *ssid = "wangyuan1";
const char *password = "wangyuan123$";
const char *websocket_server = "ws://39.106.41.164:3000/esp"; // 替换为你的服务器地址
const char *deviceId = "esp01";                               // 每个设备不同
// const char *serverUrl = "http://192.168.1.245:3000/command";
// const char *logServerUrl = "http://192.168.1.245:3000/log";

// ======== 全局对象 ========
WebsocketsClient ws;
BleComboKeyboard Keyboard("MyESP32_Combo");
BleComboMouse Mouse(&Keyboard);

// ======== 连接管理变量 ========
unsigned long lastHeartbeatTime = 0;
unsigned long heartbeatInterval = 15000; // 每 15 秒发送一次心跳
unsigned long lastReconnectAttempt = 0;
unsigned long reconnectInterval = 5000; // 每 5 秒重连尝试一次
bool isConnected = false;

// ======== 辅助状态变量 ========
String lastCommand = ""; // 记录上一次执行的命令
int httpFailCount = 0;   // 连续 HTTP 失败计数
// =========蓝牙连接判断
unsigned long lastBleCheck = 0;
const unsigned long BLE_CHECK_INTERVAL = 2000; // 2秒

// =======连接 WiFi =======
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
    Serial.println("WiFi 已重新连接");
    Serial.print("IP 地址: ");
    Serial.println(WiFi.localIP());
    httpFailCount = 0; // 重置错误计数
  }
  else
  {
    Serial.println("WiFi 连接失败，稍后重试...");
  }
}

// ======== 连接websocket ========
void connectWebSocket()
{
  if (ws.available())
  {
    Serial.println("WebSocket 已经连接，跳过连接");
    return;
  }

  Serial.println("连接 WebSocket...");

  if (ws.connect(websocket_server))
  {
    isConnected = true;
    Serial.println("WebSocket 已连接");
    ws.send("{\"type\":\"register\",\"deviceId\":\"" + String(deviceId) + "\"}");
    sendLog("设备上线");
  }
  else
  {
    isConnected = false;
    Serial.println("WebSocket 连接失败");
  }
}

// ======== 心跳机制 ========
void sendHeartbeat()
{
  if (millis() - lastHeartbeatTime > heartbeatInterval)
  {
    lastHeartbeatTime = millis();
    if (isConnected)
    {
      // ws.send("{\"type\":\"ping\",\"deviceId\":\"" + String(deviceId) + "\"}");
      ws.ping(); // 由库发送 WebSocket 控制帧 ping
      Serial.println("心跳已发送");
    }
  }
}
// ======== 发送结果到服务器 ========
// ==== 发送执行结果 ====
void sendResult(const String &result)
{
  if (isConnected)
  {
    String json = "{\"type\":\"result\",\"deviceId\":\"" + String(deviceId) + "\",\"data\":\"" + result + "\"}";
    ws.send(json);
  }
  Serial.println("Result: " + result);
}

void parseAndExecuteCommand(const String &msg)
{
  Serial.println("收到服务器消息: " + msg);

  String trimmedMsg = msg;
  trimmedMsg.trim();

  StaticJsonDocument<512> doc; // 512 字节栈空间
  DeserializationError error = deserializeJson(doc, trimmedMsg);
  if (error)
  {
    Serial.println("JSON 解析失败");
    return;
  }
  // 读取 type
  const char *type = doc["type"];
  if (!type)
  {
    Serial.println("未找到 type 字段");
    return;
  }

  // 仅处理 type 为 cmd
  if (strcmp(type, "cmd") == 0)
  {
    const char *action = doc["action"];
    if (action)
    {
      Serial.println("执行命令: " + String(action));
      handleCommand(String(action)); // 调用你的命令处理函数
    }
    else
    {
      Serial.println("未找到 action 字段");
    }
  }
  else
  {
    Serial.println("type 不是 cmd，不执行操作");
  }
}

// ======== WebSocket 回调 ========
void setupWebSocketCallbacks()
{
  ws.onMessage([](WebsocketsMessage message)
               {
                 String msg = message.data();
                 parseAndExecuteCommand(msg); });

  ws.onEvent([](WebsocketsEvent event, String data)
      {switch (event) {
      case WebsocketsEvent::ConnectionOpened:
        Serial.println("✅ WebSocket 已连接");
        break;

      case WebsocketsEvent::ConnectionClosed:
        Serial.println("❌ WebSocket 连接关闭");
        break;

      case WebsocketsEvent::GotPing:
        Serial.println("📡 收到服务器 Ping，自动回复 Pong");
        ws.pong();
        break;

      case WebsocketsEvent::GotPong:
        Serial.println("🔁 收到服务器 Pong");
        break;
    } });
}
void setup()
{

  Serial.begin(115200);
  delay(1000);
  Serial.println("启动 ESP32...");

  // 连接 WiFi
  connectWiFi();

  // ===== BLE 初始化 =====
  Keyboard.begin();
  Mouse.begin();
  Serial.println("BLE Combo启动完成，设备名：MyESP32_Combo");

  // ===== 初始化命令模块 =====
  setupCommands();

  // ===== WebSocket 初始化 =====
  setupWebSocketCallbacks();

  connectWebSocket();
}

void loop()
{
  // 检查WIFI是否连接
  if (WiFi.status() != WL_CONNECTED)
  {
    connectWiFi();
    delay(1000);
    return;
  }
  // 检查 BLE 连接状态
  unsigned long now = millis();

  if (now - lastBleCheck >= BLE_CHECK_INTERVAL)
  {
    lastBleCheck = now;
    if (!Keyboard.isConnected())
    { // 使用 Keyboard 或 Mouse 都行
      Serial.println("BLE 未连接手机");
    }
  }
  // WebSocket
  if (ws.available())
  {
    ws.poll();
    sendHeartbeat();
  }
  else
  {
    // 自动重连逻辑
    if (now - lastReconnectAttempt > reconnectInterval)
    {
      lastReconnectAttempt = now;
      connectWebSocket();
    }
  }

  delay(10);
}