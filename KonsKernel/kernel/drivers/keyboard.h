// kernel/drivers/keyboard.h
#ifndef KERNEL_DRIVERS_KEYBOARD_H
#define KERNEL_DRIVERS_KEYBOARD_H

#include <stdint.h>

// Scancode Handling
void keyboard_handler(void);
void handle_scancode(uint8_t scancode);

// Keyboard Zustand
extern int shift_pressed;
extern int ctrl_pressed;
extern int alt_pressed;
extern int caps_lock;

#endif
