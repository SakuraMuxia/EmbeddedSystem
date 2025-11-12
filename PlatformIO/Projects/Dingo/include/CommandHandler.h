#pragma once
#include <Arduino.h>
#include <BleCombo.h> 

// 外部传入 BLE 鼠标实例
void setupCommands();
void handleCommand(const String &command);