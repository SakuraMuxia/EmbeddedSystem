#include "CommandHandler.h"
#include "HttpReporter.h"

// 模块内部变量
static std::map<String, std::function<void()>> commandMap;
static String lastCommand = "";
static BleMouse *bleMousePtr = nullptr;

void setupCommands(BleMouse &mouse)
{
    bleMousePtr = &mouse;

    commandMap["click"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("BLE 未连接手机");
            return;
        }
        bleMousePtr->click(MOUSE_LEFT);
        Serial.println("执行点击");
        sendLogToServer("执行点击");
    };
    commandMap["rmtask"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("BLE 未连接手机");
            return;
        }
        // 复位到左上角
        // 因为move(x,y)只能 -128~127，所以分多步移动
        int totalMove = 1000; // 想移动的总距离
        int step = 30;// 每步最大值
        int remaining = totalMove;

        while (remaining > 0)
        {
            int moveStep = remaining > step ? step : remaining;
            bleMousePtr->move(-moveStep, -moveStep); // 每次移动都在合法范围
            remaining -= moveStep;
            delay(5); // 给设备处理时间
        }
        delay(200);
        // 先移动到屏幕中间
        int totalX = 100; // 水平方向总移动量（向右为正）
        int totalY = 800; // 垂直方向总移动量（向下为正）

        // 先横向移动
        int remainingX = totalX;
        while (remainingX > 0)
        {
            int moveX = remainingX > step ? step : remainingX;
            bleMousePtr->move(moveX, 0);
            remainingX -= moveX;
            delay(20);
        }

        // 再纵向移动
        int remainingY = totalY;
        while (remainingY > 0)
        {
            int moveY = remainingY > step ? step : remainingY;
            bleMousePtr->move(0, moveY);
            remainingY -= moveY;
            delay(30);
        }
        delay(200);
        bleMousePtr->click(MOUSE_LEFT);
        Serial.println("执行清除任务");
        sendLogToServer("执行清除任务");
    };
    commandMap["reset"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("BLE 未连接手机");
            return;
        }
        // 因为move(x,y)只能 -128~127，所以分多步移动
        int totalMove = 1000; // 想移动的总距离
        int step = 30;       // 每步最大值
        int remaining = totalMove;

        while (remaining > 0)
        {
            int moveStep = remaining > step ? step : remaining;
            bleMousePtr->move(-moveStep, -moveStep); // 每次移动都在合法范围
            remaining -= moveStep;
            delay(5); // 给设备处理时间
        }

        Serial.println("复位完成，指针已在左上角");
        sendLogToServer("复位完成，指针已在左上角");
    };

    commandMap["unlock"] = []()
    {
        if (!bleMousePtr->isConnected())
        {
            Serial.println("⚠️ BLE 未连接手机");
            return;
        }
        // 复位到左上角
        // 因为move(x,y)只能 -128~127，所以分多步移动
        int totalMove = 1000; // 想移动的总距离
        int step = 30;       // 每步最大值
        int remaining = totalMove;

        while (remaining > 0)
        {
            int moveStep = remaining > step ? step : remaining;
            bleMousePtr->move(-moveStep, -moveStep); // 每次移动都在合法范围
            remaining -= moveStep;
            delay(5); // 给设备处理时间
        }
        delay(100);
        // 先移动到屏幕中间
        int totalX = 250; // 水平方向总移动量（向右为正）
        int totalY = 400; // 垂直方向总移动量（向下为正）

        // 先横向移动
        int remainingX = totalX;
        while (remainingX > 0)
        {
            int moveX = remainingX > step ? step : remainingX;
            bleMousePtr->move(moveX, 0);
            remainingX -= moveX;
            delay(5);
        }

        // 再纵向移动
        int remainingY = totalY;
        while (remainingY > 0)
        {
            int moveY = remainingY > step ? step : remainingY;
            bleMousePtr->move(0, moveY);
            remainingY -= moveY;
            delay(5);
        }
        delay(100);
        // 滑动
        // 可调参数 —— 根据手机/屏幕分辨率与测试结果调整
        const int totalDistance = 300;            // 总移动像素（向上为负 y），增大可尝试更深的滑动
        const int steps = 20;                     // 拆分成多少小步（步数越多越平滑）
        const int durationMs = 300;               // 总耗时（ms），越大表示滑动越慢
        const int stepDelay = durationMs / steps; // 每一步延迟ms

        // 按下（按住左键，模拟按在屏幕上）
        bleMousePtr->press(MOUSE_LEFT);
        delay(10); // 给设备一点时间确认按下

        // 每一步移动的相对位移（y 方向为负，向上）
        int dy_step = -(totalDistance / steps);
        int dx_step = 0; // 若需轻微横向修正可修改

        for (int i = 0; i < steps; ++i)
        {
            bleMousePtr->move(dx_step, dy_step);
            delay(stepDelay);
        }

        // 释放（松开）
        bleMousePtr->release(MOUSE_LEFT);
        delay(10);

        Serial.println("unlock 执行结束（已发送拖拽）");
        sendLogToServer("unlock 执行结束（已发送拖拽）");
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
