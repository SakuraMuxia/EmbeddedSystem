#include "CommandHandler.h"

// 模块内部变量
static std::map<String, std::function<void()>> commandMap;
static String lastCommand = "" ;
static BleMouse *bleMousePtr = nullptr;

void setupCommands(BleMouse &mouse)
{
    bleMousePtr = &mouse;

    commandMap["click"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("⚠️ BLE 未连接手机");
            return;
        }
        bleMousePtr->click(MOUSE_LEFT);
        Serial.println("🖱 执行点击");
    };

    commandMap["right_click"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("⚠️ BLE 未连接手机");
            return;
        }
        bleMousePtr->click(MOUSE_RIGHT);
        Serial.println("🖱 执行右键点击");
    };

    commandMap["ding"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("⚠️ BLE 未连接手机");
            return;
        }

        Serial.println("🟢 执行打卡鼠标操作");

        const int steps = 30;  // 滑动步数
        const int deltaY = 10; // 每步移动的像素

        // 1️⃣ 模拟向上滑动（类似滑动解锁）
        bleMousePtr->press(MOUSE_LEFT); // 按下左键
        for (int i = 0; i < steps; i++)
        {
            bleMousePtr->move(0, -deltaY); // 向上滑动
            delay(15);
        }
        bleMousePtr->release(MOUSE_LEFT); // 松开左键
        Serial.println("👆 滑动完成");

        delay(200); // 可根据实际情况调整间隔

        // 2️⃣ 模拟点击（例如确认按钮或打卡按钮）
        bleMousePtr->click(MOUSE_LEFT);
        Serial.println("🖱 执行点击");

        delay(200);

        // 3️⃣ 可选：模拟右键或其他操作
        // bleMousePtr->click(MOUSE_RIGHT);

        // 4️⃣ 回到初始位置（屏幕中心）
        for (int i = 0; i < steps; i++)
        {
            bleMousePtr->move(0, deltaY);
            delay(15);
        }
        Serial.println("🔙 回到初始位置");
    };
}

void handleCommand(const String &command)
{
    String trimmed = command;
    trimmed.trim();

    if (trimmed.isEmpty())
        return;

    if (commandMap.find(trimmed) != commandMap.end())
    {
        commandMap[trimmed]();
    }
    else
    {
        Serial.println("⚠️ 未知命令：" + trimmed);
    }
}
