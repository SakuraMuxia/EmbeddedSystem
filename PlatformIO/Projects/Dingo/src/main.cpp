#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <BleComboKeyboard.h>
#include <BleComboMouse.h>
#include <esp_task_wdt.h>
#include "CommandHandler.h"

using namespace websockets;

// ======== 配置信息 ========
const char *ssid = "wangyuan1";
const char *password = "wangyuan123$";
const char *websocket_server = "ws://39.106.41.164:3000/esp"; // 替换为你的服务器地址
                                                              // 每个设备不同
// const char *serverUrl = "http://192.168.1.245:3000/command";
// const char *logServerUrl = "http://192.168.1.245:3000/log";

// ======== 全局对象 ========
WebsocketsClient ws;
BleComboKeyboard Keyboard("MyESP32_Combo");
BleComboMouse Mouse(&Keyboard);

const int LED_PIN = 2;

// ======== LED 状态控制 ========
enum ConnState
{
  LED_OFFLINE,
  LED_WIFI_CONNECTING,
  LED_WS_CONNECTING,
  LED_ONLINE
};

ConnState currentState = LED_OFFLINE;
unsigned long lastBlinkTime = 0;
bool ledState = false;

// ======== 定时器与状态变量 ========
bool wsConnected = false;
unsigned long lastHeartbeat = 0;
unsigned long lastPongTime = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long reconnectDelay = 5000;
const unsigned long HEARTBEAT_INTERVAL = 15000;
const unsigned long PONG_TIMEOUT = 30000;
const unsigned long MAX_RECONNECT_DELAY = 60000;

// BLE 检查
unsigned long lastBleCheck = 0;
const unsigned long BLE_CHECK_INTERVAL = 2000;

// ======== Watchdog 配置 ========
#define WDT_TIMEOUT 30 // 秒
void setupWatchdog()
{
  esp_task_wdt_init(WDT_TIMEOUT, true); // 启动 WDT
  esp_task_wdt_add(NULL);               // 当前任务加入监控
}

// ======== 生成唯一设备 ID ========
String generateDeviceId()
{
  uint64_t mac = ESP.getEfuseMac(); // ESP32 唯一 MAC

  char buf[32];
  // 拆分 MAC 为 4 个块，每块 4 位 HEX
  sprintf(buf, "esp32-%04lX_%04lX_%04lX_%04lX",
          (uint16_t)(mac >> 48), // 高16位
          (uint16_t)(mac >> 32), // 次高16位
          (uint16_t)(mac >> 16), // 次低16位
          (uint16_t)(mac));      // 低16位

  return String(buf);
}

String deviceId = generateDeviceId();
// ======== LED 控制函数 ========
void updateLed()
{
  unsigned long now = millis();

  switch (currentState)
  {
  case LED_OFFLINE: // 离线：灭
    digitalWrite(LED_PIN, LOW);
    break;

  case LED_WIFI_CONNECTING: // WiFi连接中：慢闪
    if (now - lastBlinkTime > 500)
    {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = now;
    }
    break;

  case LED_WS_CONNECTING: // WS连接中：快闪
    if (now - lastBlinkTime > 200)
    {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlinkTime = now;
    }
    break;

  case LED_ONLINE: // 在线：常亮
    digitalWrite(LED_PIN, HIGH);
    break;
  }
}
void setLedState(ConnState newState)
{
  if (currentState != newState)
  {
    currentState = newState;
    lastBlinkTime = 0;
  }
}
// =======连接 WiFi =======
void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  setLedState(LED_WIFI_CONNECTING);

  Serial.println("📶 WiFi 连接中...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000)
  {
    updateLed();
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("✅ WiFi 已连接，IP: %s\n", WiFi.localIP().toString().c_str());
  }
  else
  {
    Serial.println("❌ WiFi 连接失败，尝试重新连接");
    WiFi.reconnect();
  }
}

// ======== 连接websocket ========
void connectWebSocket()
{
  if (wsConnected)
    return;
  if (WiFi.status() != WL_CONNECTED)
    return;
  setLedState(LED_WS_CONNECTING);
  Serial.println("🔌 正在连接 WebSocket...");
  if (ws.connect(websocket_server))
  {
    delay(300);
    wsConnected = true;
    lastPongTime = millis();
    reconnectDelay = 5000;
    Serial.println("✅ WebSocket 已连接");
    ws.send("{\"type\":\"register\",\"deviceId\":\"" + String(deviceId) + "\"}");
    setLedState(LED_ONLINE);
  }
  else
  {
    wsConnected = false;
    Serial.printf("❌ WebSocket 连接失败，%lu ms 后重试\n", reconnectDelay);
  }
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
               { parseAndExecuteCommand(message.data()); });

  ws.onEvent([](WebsocketsEvent event, String data)
             {
    switch (event) {
      case WebsocketsEvent::ConnectionOpened:
        Serial.println("🟢 WebSocket 打开连接");
        wsConnected = true;
        lastPongTime = millis();
        setLedState(LED_ONLINE);
        break;

      case WebsocketsEvent::ConnectionClosed:
        Serial.println("🔴 WebSocket 连接关闭");
        wsConnected = false;
        setLedState(LED_OFFLINE);
        break;

      case WebsocketsEvent::GotPing:
        ws.pong();
        Serial.println("📡 收到服务器 Ping → 已回复 Pong");
        lastPongTime = millis();
        break;

      case WebsocketsEvent::GotPong:
        lastPongTime = millis();
        Serial.println("🔁 收到服务器 Pong");
        break;

        default:
        Serial.println("⚠️ 未知 WebSocket 事件");
        break;
    } });
}

// ======== 心跳 + 假连接检测 ========
void sendHeartbeat()
{
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL)
  {
    lastHeartbeat = millis();
    if (wsConnected)
    {
      ws.ping();
      Serial.println("💓 发送心跳 Ping");
    }
  }

  if (wsConnected && millis() - lastPongTime > PONG_TIMEOUT)
  {
    Serial.println("⚠️ Pong 超时 → 假连接判定 → 重连中...");
    ws.close();
    wsConnected = false;
    setLedState(LED_OFFLINE);
  }
}

// ======== 主体 setup ========
void setup()
{

  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  setupWatchdog();

  setLedState(LED_OFFLINE);
  Serial.println("启动 ESP32...");
  // 连接 WiFi
  connectWiFi();
  // ===== BLE 初始化 =====
  Keyboard.begin();
  Mouse.begin();

  // ===== 初始化命令模块 =====
  setupCommands();

  // ===== WebSocket 初始化 =====
  setupWebSocketCallbacks();

  connectWebSocket();
}

// ======== 主循环 ========
void loop()
{
  esp_task_wdt_reset(); // ✅ 喂狗防止死循环重启
  updateLed();

  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED)
  {
    wsConnected = false;
    setLedState(LED_WIFI_CONNECTING);
    WiFi.reconnect();
    delay(200);
    return;
  }

  // BLE 检查
  if (now - lastBleCheck >= BLE_CHECK_INTERVAL)
  {
    lastBleCheck = now;
    if (!Keyboard.isConnected())
    {
      Serial.println("📱 BLE 未连接手机");
    }
  }

  // WebSocket 检查
  if (wsConnected)
  {
    ws.poll();
    sendHeartbeat();
  }
  else if (now - lastReconnectAttempt > reconnectDelay)
  {
    lastReconnectAttempt = now;
    connectWebSocket();
    if (!wsConnected && reconnectDelay < MAX_RECONNECT_DELAY)
      reconnectDelay *= 2; // 失败指数退避
  }

  delay(10);
}