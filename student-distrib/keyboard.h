#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "lib.h"

#define KEY_PORT 0x60
#define KEY_IRQ 0x01
#define KB_BUF_SIZE 128

// initialize_keyboard
void initialize_keyboard(void);

// handler c function - keyboard interrupt
void keyboard_c_handler(void);

#endif /* _KEYBOARD_H*/
