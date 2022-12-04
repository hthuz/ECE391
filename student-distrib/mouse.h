
#ifndef _MOUSE_H
#define _MOUSE_H

#include "lib.h"

#define MOUSE_PORT 0x64
#define MOUSE_IRQ 12
#define RESET_CMD 0xff
#define ENABLE_PACKET_STREM_CMD 0xf4
#define MASK_THIRD 0xdf
#define SLOW_RATE  6
#define WIDTH   80
#define HEIGHT  25


#define GET_FIRST   0x80
#define GET_SECOND  0x40
#define GET_THIRD   0x20
#define GET_FOURTH  0x10
#define GET_FIFTH   0x08
#define GET_SIXTH   0x04
#define GET_SEVENTH 0x02
#define GET_LAST    0x01

extern int32_t mouse_x;
extern int32_t mouse_y;
extern int32_t previous_x;
extern int32_t previous_y;
// initilize the mouse
void mouse_init();

// handle the mouse interrupt
void mouse_handler();

// helper functions for the two functions
uint8_t read_keyboard_port();
uint8_t read_mouse_port();
void write_mouse_port(uint8_t data);
void write_keyboard_port(uint8_t data);
void send_command(uint8_t command);
void wait_input();
void wait_output();


#endif /* _MOUSE_H*/
