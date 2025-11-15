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
void press2Esc()
{
    Keyboard.press(KEY_ESC);
    delay(100);
    Keyboard.release(KEY_ESC);
    delay(100);
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
    delay(500);
    // 移动到中心
    mouseMoveSmooth(180, 610);
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
        // 解锁屏幕
        unlockScreen();
        delay(2000);

        // 点击钉应用图标
        pressAltTab();
        delay(200);
        pressAltTab();
        delay(2000);

        // 切换到打卡界面
        for (int i = 0; i < 9; i++){
            pressTab();
            delay(200);
        }
        delay(200);
        pressEnter();
        delay(400);

        delay(4000);
        // 点击打卡按钮
        clickDing();
        delay(5000);

        // // 返回手机桌面
        // // 按 3 下 Tab 键
        for (int i = 0; i < 3; i++){
            pressTab();
            delay(200);
        }
        delay(200);
        // // 按 Enter 键
        pressEnter();
        delay(200);
        // pressEsc();
        delay(400);
        press2Esc();
        sendLog("clockIn","success","已完成打卡操作");


    }else if (command == "unlock")
    {
        unlockScreen();
        sendLog("unlock","success","屏幕已解锁");
    }
    else if (command == "2altTab")
    {
        pressAltTab();
        delay(200);
        pressAltTab();
        sendLog("2altTab","success","已切换到应用");
    }
    else if (command == "swipeDing")
    {
        clickDing();
        sendLog("swipeDing","success","已点击钉");
    }
    else if (command == "switchDk9")
    {   
        // 按 9 下 Tab 键
        for (int i = 0; i < 9; i++){
            pressTab();
            delay(200);
        }
        delay(200);
        // 按 Enter 键
        pressEnter();
        sendLog("switchDk9","success","已切换到打卡界面");
    }
    
    else if (command == "switchFh")
    {   
        // 按 8 下 Tab 键
        for (int i = 0; i < 6; i++){
            pressTab();
            delay(200);
        }
        delay(200);
        // 按 Enter 键
        pressEnter();
        delay(200);
        pressEsc();
        delay(400);
        press2Esc();
        sendLog("switchFh","success","已切换到分红界面");
        
    }
    else if (command == "altTab")
    {
        pressAltTab();
        sendLog("altTab","success","已切换到上一个应用");
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
        sendLog("enter","success","已发送回车键");
    }
    else if (command == "esc")
    {
        pressEsc();
        sendLog("esc","success","已发送Esc键");
    }
    else if (command == "2esc")
    {
        pressEsc();
        delay(200);
        pressEsc();
        sendLog("2esc","success","已发送两次Esc键");
    }
    else if (command == "backspace")
    {
        pressBackspace();
        sendLog("backspace","success","已发送退格键");
    }
    else if (command == "tab")
    {
        pressTab();
        sendLog("tab","success","已发送Tab键");
    }
    else if (command == "clickMouse")
    {
        clickMouse();
        sendLog("clickMouse","success","已点击鼠标");
    }
    else if (command == "startMenu")
    {
        openStartMenu();
        sendLog("startMenu","success","已打开开始菜单");
    }
    else if (command == "minimizeWindow")
    {
        minimizeCurrentWindow();
        sendLog("minimizeWindow","success","已最小化当前窗口");
    }
    else
    {
        Serial.println("未知命令: " + command);
        sendLog(command,"fail", "未知命令");
    }
}