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
// 让 CommandHandler.cpp 可以使用 main.ino 里的 sendLog()
void sendLog(const String &command,const String &result,const String &message);
// 处理服务器下发的命令
void handleCommand(const String &command);

// 发送日志到服务器
void sendLog(const String &log);

// 按下alt tab
void pressAltTab();
// 按下解锁键
void unlockScreen();
// 方向键
void pressUpArrow();
void pressDownArrow();
void pressLeftArrow();
void pressRightArrow();
// 确认键
void pressEnter();
// 鼠标点击
void clickMouse();
// 开始菜单
void openStartMenu();
// tab键
void pressTab();
// esc键
void pressEsc();
// backspace键
void pressBackspace();
// 最小化窗口
void minimizeCurrentWindow();

#endif