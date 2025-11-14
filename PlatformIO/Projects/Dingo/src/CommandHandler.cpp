#include "CommandHandler.h"
#include <BleComboKeyboard.h>
#include <BleComboMouse.h>

// 可以在这里初始化命令映射或日志
void setupCommands()
{
    Serial.println("命令模块已初始化");
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

// 辅助函数：模拟滑动手势 向上为 负，向下为正，向左为负，向右为正
void swipe(int dx, int dy, int steps = 15, int stepDelay = 20)
{
    if (!Keyboard.isConnected())
        return;
    Mouse.press(MOUSE_LEFT);
    mouseMoveSmooth(dx, dy, steps, stepDelay);
    Mouse.release(MOUSE_LEFT);
}

// 模拟 Alt+Tab 切换窗口
void pressAltTab()
{
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_TAB);
    delay(150);
    Keyboard.release(KEY_TAB);
    Keyboard.release(KEY_LEFT_ALT);
}

// 解锁屏幕
void unlockScreen()
{
    Mouse.click(MOUSE_LEFT);
    delay(1000);
    // 复位
    mouseMoveSmooth(-600, -1000);
    delay(300);
    // 移动到中心
    mouseMoveSmooth(300, 500);
    // 滑动
    swipe(0, -400); // 向上滑动
}

// 方向右
void pressRightArrow()
{
    Keyboard.press(KEY_RIGHT_ARROW);
    delay(100);
    Keyboard.release(KEY_RIGHT_ARROW);
}
// 方向左
void pressLeftArrow()
{
    Keyboard.press(KEY_LEFT_ARROW);
    delay(100);
    Keyboard.release(KEY_LEFT_ARROW);
}

// 方向右
void pressUpArrow()
{
    Keyboard.press(KEY_UP_ARROW);
    delay(100);
    Keyboard.release(KEY_UP_ARROW);
}

// 方向xia
void pressDownArrow()
{
    Keyboard.press(KEY_DOWN_ARROW);
    delay(100);
    Keyboard.release(KEY_DOWN_ARROW);
}

void pressEnter()
{
    Keyboard.press(KEY_RETURN);
    delay(100);
    Keyboard.release(KEY_RETURN);
}

void pressEsc()
{
    Keyboard.press(KEY_ESC);
    delay(100);
    Keyboard.release(KEY_ESC);
}

void pressBackspace()
{
    Keyboard.press(KEY_BACKSPACE);
    delay(100);
    Keyboard.release(KEY_BACKSPACE);
}

void pressTab()
{
    Keyboard.press(KEY_TAB);
    delay(100);
    Keyboard.release(KEY_TAB);
}

void clickMouse()
{
    Mouse.click(MOUSE_LEFT);
}

void openStartMenu()
{
    Keyboard.press(KEY_LEFT_GUI);
    delay(100);
    Keyboard.release(KEY_LEFT_GUI);
}

void closeCurrentWindow()
{
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('W');
    delay(100);
    Keyboard.release('W');
    Keyboard.release(KEY_LEFT_GUI);
}

void minimizeCurrentWindow()
{
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('D');
    delay(100);
    Keyboard.release('D');
    Keyboard.release(KEY_LEFT_GUI);
}

void clickDing()
{
    // 复位
    mouseMoveSmooth(-600, -1000);
    delay(300);
    // 移动到中心
    mouseMoveSmooth(180, 650);
    delay(1000);
    Mouse.click(MOUSE_LEFT);
}

// 处理命令
void handleCommand(const String &command)
{
    if (command == "")
    {
        return;
    }
    // 鼠标操作
    if (command == "clockIn")
    {
        
    }else if (command == "unlock")
    {
        unlockScreen();
        sendLog("unlock","success","屏幕已解锁");
    }
    else if (command == "altTab")
    {
        pressAltTab();
    }
    else if (command == "upArrow")
    {
        pressUpArrow();
    }
    else if (command == "downArrow")
    {
        pressDownArrow();
    }
    else if (command == "leftArrow")
    {
        pressLeftArrow();
    }
    else if (command == "rightArrow")
    {
        pressRightArrow();
    }
    else if (command == "enter")
    {
        pressEnter();
    }
    else if (command == "esc")
    {
        pressEsc();
    }
    else if (command == "backspace")
    {
        pressBackspace();
    }
    else if (command == "tab")
    {
        pressTab();
    }
    else if (command == "clickMouse")
    {
        clickMouse();
    }
    else if (command == "startMenu")
    {
        openStartMenu();
    }
    else if (command == "minimizeWindow")
    {
        minimizeCurrentWindow();
    }
    else
    {
        Serial.println("未知命令: " + command);
        sendLog(command,"fail", "未知命令");
    }
}