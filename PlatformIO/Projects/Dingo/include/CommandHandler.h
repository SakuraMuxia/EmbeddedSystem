#pragma once
#include <Arduino.h>
#include <map>
#include <functional>
#include <BleMouse.h>

// 外部传入 BLE 鼠标实例
void setupCommands(BleMouse &mouse);
void handleCommand(const String &command);