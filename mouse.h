
#ifndef _MOUSE_H
#define _MOUSE_H

#include "lib.h"

#define MOUSE_PORT 0x64
#define MOUSE_IRQ 12

void mouse_init();
void mouse_handler();


uint8_t read_keyboard_port();
uint8_t read_mouse_port();
void write_mouse_port(uint8_t data);
void write_keyboard_port(uint8_t data);
void send_command(uint8_t command);
void wait_input();
void wait_output();
#endif /* _MOUSE_H*/
