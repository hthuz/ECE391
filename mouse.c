
#include "mouse.h"
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

int32_t mouse_x = 0;
int32_t mouse_y = 0;
int32_t previous_x = 0;
int32_t previous_y = 0;



uint8_t read_mouse_port(){
    uint8_t data;
    wait_input();
    data = inb(MOUSE_PORT);
    return data;
}

uint8_t read_keyboard_port(){
    uint8_t data;
    wait_input();
    data = inb(KEY_PORT);
    return data;
}

void write_mouse_port(uint8_t data){
    wait_output();
    outb(data,MOUSE_PORT);
    return;
}

void write_keyboard_port(uint8_t data){
    wait_output();
    outb(data,KEY_PORT);
    return;
}


void send_command(uint8_t command){
    wait_output();
    outb(0xd4,MOUSE_PORT);
    wait_output();
    outb(command,KEY_PORT);
    // printf("send command success\n");
    // return;
}
/* 
 *  wait_port()
 *  DESCRIPTION: All output to port 0x60 or 0x64 
 *              must be preceded by waiting for bit 1 (value=2) 
 *              of port 0x64 to become clear. Similarly, 
 *              bytes cannot be read from port 0x60 until 
 *              bit 0 (value=1) of port 0x64 is set. 
 *              See PS2 Keyboard for further details.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: 
 */
void wait_input(){
    while(1){
        if ( (inb(MOUSE_PORT)& 0x01) !=0 ) break;
    }
    // printf("wait input success\n");
    // int i = 100000;
    // while(i-- && inb(MOUSE_PORT)&1);
    // printf("wait input success\n");
}


/* 
 *  wait_port()
 *  DESCRIPTION: All output to port 0x60 or 0x64 
 *              must be preceded by waiting for bit 1 (value=2) 
 *              of port 0x64 to become clear. Similarly, 
 *              bytes cannot be read from port 0x60 until 
 *              bit 0 (value=1) of port 0x64 is set. 
 *              See PS2 Keyboard for further details.
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: 
 */
void wait_output(){
    while(1){
        if ((inb(MOUSE_PORT)& 0x02)!=1) break;
    }
    printf("wait output success\n");
}


/* 
 *  mouse_init()
 *  DESCRIPTION: init_mouse
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS: 
 */
void mouse_init(){
    // reset
    send_command(0xff);
    // write_mouse_port(0xa8);
    // Set Compaq Status
    write_mouse_port(0x20);
    // Very next byte returned should be the Status byte, 
    // write status
    uint8_t status = read_keyboard_port();
    status = ((status | 0x02 ) & (0xdf) );
    write_mouse_port(0x60);
    write_keyboard_port(status);

    // mouse send packets
    send_command(0xf4);

    set_background_green(mouse_x,mouse_y);
    // enable IRQ12
    enable_irq(MOUSE_IRQ);
}



void mouse_handler(){
    // printf("call handler\n");
    send_eoi(MOUSE_IRQ);
    // int32_t i;
    uint8_t packet = read_keyboard_port();
    int32_t x_move = read_keyboard_port();
    int32_t y_move = read_keyboard_port();
    uint8_t y_over = ( packet & 0x80) ;
    uint8_t x_over = ( packet & 0x40) ;
    uint8_t y_sign = ( packet & 0x20) ;
    uint8_t x_sign = ( packet & 0x10) ;
    uint8_t al_one = ( packet & 0x08) ;
    // uint8_t m_button = ( packet & 0x04) ;
    // uint8_t r_button = ( packet & 0x02) ;
    uint8_t l_button = ( packet & 0x01) ;

    if ( x_over == 0x40 || y_over == 0x80 || al_one == 0)  return;
    // for (i=0 ;i<=10000; i++ ){
    //     x_move=x_move;
    //     y_move=y_move;
    // }
    x_move=x_move/3;
    y_move=y_move/3;
    x_move = ( x_move - (x_sign<<4)) ;
    y_move = ( y_move - (y_sign<<4)) ;
    
    if( x_move + mouse_x < 0) mouse_x = 0;
    else if ( x_move + mouse_x > 79) mouse_x = 79;
    else  mouse_x = mouse_x + x_move;

    if( y_move + mouse_y < 0) mouse_y = 0;
    else if ( y_move + mouse_y > 24) mouse_y = 24;
    else  mouse_y = mouse_y + y_move;

    set_background_black(previous_x,previous_y);
    set_background_green(mouse_x,mouse_y);

    previous_x = mouse_x;
    previous_y = mouse_y;
    if ( l_button == 0x01 ) terminal_switch(0);
}



