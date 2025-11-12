#include "CommandHandler.h"
#include <BleComboKeyboard.h>
#include <BleComboMouse.h>

// 可以在这里初始化命令映射或日志
void setupCommands()
{
    Serial.println("命令模块已初始化");
}

// 发送日志函数
void sendLog(const String &log)
{
    if (ws.available())
    {
        ws.send("{\"type\":\"log\",\"message\":\"" + log + "\"}");
    }
    Serial.println("Log: " + log);
}


// 辅助函数：平滑鼠标移动
void mouseMoveSmooth(int dx, int dy, int steps = 10, int stepDelay = 20)
{
    for (int i = 0; i < steps; i++)
    {
        Mouse.move(dx / steps, dy / steps);
        delay(stepDelay);
    }
}

// 辅助函数：模拟滑动手势
void swipe(int dx, int dy, int steps = 15, int stepDelay = 20)
{
    if (!Keyboard.isConnected())
        return;
    Mouse.press(MOUSE_LEFT);
    mouseMoveSmooth(dx, dy, steps, stepDelay);
    Mouse.release(MOUSE_LEFT);
}
// 处理命令
void handleCommand(const String &command)
{
    if (command == "")
    {
        return;
    }
    // 鼠标操作
    if (command == "click")
    {
        Mouse.click(MOUSE_LEFT);
    }
    else if (command == "MOVE_DOWN")
    {
        // 平滑移动，避免偏差
        for (int i = 0; i < 10; i++)
        {
            Mouse.move(0, 10);
            delay(20);
        }
    }
    else if (command == "MOVE_UP")
    {
        for (int i = 0; i < 10; i++)
        {
            Mouse.move(0, -10);
            delay(20);
        }
    }

    // 键盘操作
    else if (command == "down")
    {
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(100);
        Keyboard.release(KEY_RIGHT_ARROW);
    }
    else if (command == "right")
    {
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(100);
        Keyboard.release(KEY_RIGHT_ARROW);
    }

    else if (command == "menu")
    {
        Keyboard.press(KEY_LEFT_GUI);
        delay(100);
        Keyboard.release(KEY_LEFT_GUI);
    }
    else if (command == "rmtask")
    {
        Keyboard.press(KEY_LEFT_ALT);   // 按下 GUI/Meta 键
        Keyboard.press(KEY_TAB);        // 按下 Tab 键
        delay(150);                      // 保持一小段时间
        Keyboard.release(KEY_TAB);      // 松开 Tab 键
        Keyboard.release(KEY_LEFT_ALT); // 松开 GUI 键
    }
    else if (command == "tab")
    {
        Keyboard.press(KEY_TAB);   // 按下 GUI/Meta 键
        delay(150);                      // 保持一小段时间
        Keyboard.release(KEY_TAB);      // 松开 Tab 键
    }
    else if (command == "esc")
    {
        Keyboard.press(KEY_ESC);   // 按下 GUI/Meta 键
        delay(150);                      // 保持一小段时间
        Keyboard.release(KEY_ESC);      // 松开 Tab 键
    }
    else if (command == "enter")
    {
        Keyboard.press(KEY_RETURN);   // 按下 GUI/Meta 键
        delay(150);                      // 保持一小段时间
        Keyboard.release(KEY_RETURN);      // 松开 Tab 键
    }
    // 未知命令
    else
    {
        Serial.println("未知命令: " + command);
    }
}