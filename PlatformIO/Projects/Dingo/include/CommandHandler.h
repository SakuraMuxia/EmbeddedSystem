#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

extern WebsocketsClient ws;
extern class BleComboKeyboard Keyboard;
extern class BleComboMouse Mouse;
// 初始化命令模块
void setupCommands();

// 处理服务器下发的命令
void handleCommand(const String &command);

// 发送日志到服务器
void sendLog(const String &log);

#endif