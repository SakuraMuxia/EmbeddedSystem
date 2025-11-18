#pragma once
#include <Arduino.h>

bool wsConnect();
bool wsIsConnected();
bool wsSendFrame(uint8_t* data, size_t len);
void wsLoop();