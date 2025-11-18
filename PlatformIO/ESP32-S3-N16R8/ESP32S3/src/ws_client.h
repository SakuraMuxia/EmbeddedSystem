#pragma once
#include <Arduino.h>
// 全局流控制
extern bool streaming; 
bool wsConnect();
bool wsIsConnected();
bool wsSendFrame(uint8_t* data, size_t len);
void wsLoop();