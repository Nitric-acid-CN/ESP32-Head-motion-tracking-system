#include <USB.h>
#include <USBHIDMouse.h>
#include <USBHIDKeyboard.h>

extern USBHIDMouse Mouse;
extern USBHIDKeyboard Keyboard;

extern int mouse_movement;

uint8_t getKeyCode(int selected_key);

void processMouseMovement();
void sendKey();