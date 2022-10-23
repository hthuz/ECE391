#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"

#define KEY_PORT 0x60
#define KEY_IRQ 0x01
#define KB_BUF_SIZE 128

#define CURSOR_PORT 0x3D4
#define CURSOR_DATA 0x3D5

// initialize_keyboard
void initialize_keyboard(void);

// handler c function - keyboard interrupt
void keyboard_c_handler(void);

// use to differentiate shift and CAPSLOCK
int is_alphabet(unsigned char scancode);

// used to scroll the screen by one line
int scroll_one_line();

// functions for cursor control
// Refer to https://wiki.osdev.org/Text_Mode_Cursor
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);

extern unsigned char kb_buf[KB_BUF_SIZE];
extern int kb_buf_length;

// if enter is presed, used to tell if need to terminal read
extern int enter_pressed;

#endif /* _KEYBOARD_H*/
