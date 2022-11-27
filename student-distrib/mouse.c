
#include "mouse.h"
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

int32_t mouse_x = 0;
int32_t mouse_y = 0;
int32_t previous_x = 0;
int32_t previous_y = 0;
// int32_t turn_term_flag=1;



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
}

void write_keyboard_port(uint8_t data){
    wait_output();
    outb(data,KEY_PORT);
}


void send_command(uint8_t command){
    wait_output();
    outb(0xd4,MOUSE_PORT);
    wait_output();
    outb(command,KEY_PORT);
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
        // for bit 1
        if ( (inb(MOUSE_PORT)& 0x01) !=0 ) break;
    }
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
        // for bit 2
        if ((inb(MOUSE_PORT)& 0x02)!=1) break;
    }
    // printf("wait output success\n");
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
    send_command(RESET_COM);
    // Set Compaq Status
    write_mouse_port(0x20);
    // Very next byte returned should be the Status byte, 
    // |Y overflow	|X overflow	 |Y sign bit  |X sign bit	
    // |Always 1	|Middle Btn	 |Right Btn	  |Left Btn
    uint8_t status = read_keyboard_port();
    status = ((status | 0x02 ) & MASK_THIRD );  // magic number for set status
    // write status
    write_mouse_port(KEY_PORT);
    write_keyboard_port(status);

    // mouse send packets
    send_command(PACKET_STR_COM);

    set_background_green(mouse_x,mouse_y);
    // enable IRQ12
    enable_irq(MOUSE_IRQ);
}



void mouse_handler(){
    // printf("call handler\n");
    send_eoi(MOUSE_IRQ);
    int32_t x_move,y_move;
    uint8_t packet = read_keyboard_port();
    uint8_t y_over = ( packet & GET_FIRST) ;
    uint8_t x_over = ( packet & GET_SECOND) ;
    uint8_t y_sign = ( packet & GET_THIRD) ;
    uint8_t x_sign = ( packet & GET_FOURTH) ;
    uint8_t al_one = ( packet & GET_FIFTH) ;
    // uint8_t m_button = ( packet & GET_SIXTH) ;
    // uint8_t r_button = ( packet & GET_SEVENTH) ;
    uint8_t l_button = ( packet & GET_LAST) ;

    if ( x_over == GET_SECOND || y_over == GET_FIRST || al_one == 0)  return;

    x_move = read_keyboard_port();
    x_move = ( x_move - (x_sign<<4)) ;  //magic number to get x_sign

    y_move = read_keyboard_port();
    y_move = ( y_move - (y_sign<<3)) ;  // magic number to get y_sign
    y_move = -y_move;       // the y_move is different from convention

    // slow down the rate of the "cursor"
    x_move=x_move/SLOW_RATE;
    y_move=y_move/SLOW_RATE;

    if( x_move + mouse_x < 0) mouse_x = 0;
    else if ( x_move + mouse_x >= WIDTH) mouse_x = WIDTH-1;
    else  mouse_x = mouse_x + x_move;

    if( y_move + mouse_y < 0) mouse_y = 0;
    else if ( y_move + mouse_y >= HEIGHT) mouse_y = HEIGHT-1;
    else  mouse_y = mouse_y + y_move;

    if ( mouse_x!=previous_x || mouse_y!= previous_y){
        set_background_green(mouse_x,mouse_y);
        set_background_black(previous_x,previous_y);
    }
    // set_background_black(previous_x,previous_y);
    previous_x = mouse_x;
    previous_y = mouse_y;
    // set_background_black(previous_x,previous_y);
    // if ( r_button == GET_SEVENTH ) 
    //     terminal_switch(0);
    // if ( m_button == GET_SIXTH) 
    //     terminal_switch(1);
    if ( l_button == GET_LAST ) {
        set_background_black(mouse_x,mouse_y);
        terminal_switch(0);
    }
}



