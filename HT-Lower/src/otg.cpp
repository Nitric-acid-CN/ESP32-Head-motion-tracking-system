#include "otg.h"
#include "ui_main.h"
#include "config.h"
#include "wireless.h"
#include "ppm.h"

USBHIDMouse Mouse;
USBHIDKeyboard Keyboard;

int perStep = 20;

void processMouseMovement() {
  int x = 0, y = 0;

  // 根据用户选择处理X轴
  switch (m_x) {
    case 0:  // Yaw 正向
      x = map(yaw, -180, 180, -perStep, perStep);
      break;
    case 1:  // Yaw 反向
      x = map(yaw, -180, 180, perStep, -perStep);
      break;
    case 2:  // Pitch 正向
      x = map(pitch, -90, 90, -perStep, perStep);
      break;
    case 3:  // Pitch 反向
      x = map(pitch, -90, 90, perStep, -perStep);
      break;
    case 4:  // Roll 正向
      x = map(roll, -90, 90, -perStep, perStep);
      break;
    case 5:  // Roll 反向
      x = map(roll, -90, 90, perStep, -perStep);
      break;
  }

  // 根据用户选择处理Y轴
  switch (m_y) {
    case 0:  // Yaw 正向
      y = map(yaw, -180, 180, -perStep, perStep);
      break;
    case 1:  // Yaw 反向
      y = map(yaw, -180, 180, perStep, -perStep);
      break;
    case 2:  // Pitch 正向
      y = map(pitch, -90, 90, -perStep, perStep);
      break;
    case 3:  // Pitch 反向
      y = map(pitch, -90, 90, perStep, -perStep);
      break;
    case 4:  // Roll 正向
      y = map(roll, -90, 90, -perStep, perStep);
      break;
    case 5:  // Roll 反向
      y = map(roll, -90, 90, perStep, -perStep);
      break;
  }

  // 使用灵敏度调整移动速度
  x = x * (mouse_sens / 5.0);
  y = y * (mouse_sens / 5.0);

  // 发送鼠标移动数据
  Mouse.move(x, y);
}

void sendKey() {
    uint8_t keycode = getKeyCode(m_z);  // 获取对应的keycode
    if (keycode != 0) {
        Keyboard.press(keycode);  // 按下按键
        delay(100);               // 保持按键
        Keyboard.release(keycode); // 释放按键
    }
}

// 将dropdown的索引映射为USBHIDKeyboard的按键代码
uint8_t getKeyCode(int selected_key) {
    if (selected_key >= 0 && selected_key <= 25) {
        return 'A' + selected_key;  // A-Z
    } else if (selected_key >= 26 && selected_key <= 35) {
        return '0' + (selected_key - 26);  // 0-9
    } else {
        switch (selected_key) {
            case 36: return KEY_LEFT_CTRL;
            case 37: return KEY_LEFT_SHIFT;
            case 38: return KEY_LEFT_ALT;
            case 39: return KEY_RIGHT_CTRL;
            case 40: return KEY_RIGHT_SHIFT;
            case 41: return KEY_RIGHT_ALT;
            case 42: return KEY_UP_ARROW;
            case 43: return KEY_DOWN_ARROW;
            case 44: return KEY_LEFT_ARROW;
            case 45: return KEY_RIGHT_ARROW;
            case 46: return KEY_BACKSPACE;
            case 47: return KEY_TAB;
            case 48: return KEY_RETURN;
            case 49: return KEY_ESC;
            case 50: return KEY_INSERT;
            case 51: return KEY_DELETE;
            case 52: return KEY_PAGE_UP;
            case 53: return KEY_PAGE_DOWN;
            case 54: return KEY_HOME;
            case 55: return KEY_END;
            case 56: return KEY_CAPS_LOCK;
            case 57: return KEY_F1;
            case 58: return KEY_F2;
            case 59: return KEY_F3;
            case 60: return KEY_F4;
            case 61: return KEY_F5;
            case 62: return KEY_F6;
            case 63: return KEY_F7;
            case 64: return KEY_F8;
            case 65: return KEY_F9;
            case 66: return KEY_F10;
            case 67: return KEY_F11;
            case 68: return KEY_F12;
            default: return 0;  // 不支持的按键返回 0
        }
    }
}