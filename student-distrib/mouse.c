
#include "mouse.h"
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

// Initially at center
int32_t mouse_x = 40;
int32_t mouse_y = 12;
int32_t previous_x = 40;
int32_t previous_y = 12;


uint8_t read_mouse_port()
{
    uint8_t data;
    wait_input();
    data = inb(MOUSE_PORT);
    return data;
}

uint8_t read_keyboard_port()
{
    uint8_t data;
    wait_input();
    data = inb(KEY_PORT);
    return data;
}

void write_mouse_port(uint8_t data)
{
    wait_output();
    outb(data, MOUSE_PORT);
}

void write_keyboard_port(uint8_t data)
{
    wait_output();
    outb(data, KEY_PORT);
}

void send_command(uint8_t command)
{
    wait_output();
    outb(0xd4, MOUSE_PORT);

    wait_output();
    outb(command, KEY_PORT);
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
void wait_input()
{
    while (1)
    {
        // for bit 1
        if (( inb(MOUSE_PORT) & 0x01) != 0)
            break;
    }
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
void wait_output()
{
    while (1)
    {
        // for bit 2
        if ((inb(MOUSE_PORT) & 0x02) != 1)
            break;
    }
}

/*
 *  mouse_init()
 *  DESCRIPTION: init_mouse
 *  INPUTS: none
 *  OUTPUTS: none
 *  RETURN VALUE: none
 *  SIDE EFFECTS:
 */
void mouse_init()
{

    // From keyboard or mouse
    
    send_command(RESET_CMD);

    // Set Compaq Status
    // Send "Get Compaq Status Byte"
    write_mouse_port(0x20);

    uint8_t status;
    status = read_keyboard_port();

    // Set bit 1 to enable IRQ 12
    status |= 0x02;
    // Clear bit 5 to disable Mouse Clock
    status &= (~0x20);

    // Send "Set Compaq Status"
    write_mouse_port(KEY_PORT);
    write_keyboard_port(status);

    // Enable Packet Streaming
    send_command(ENABLE_PACKET_STREM_CMD);
    // Acknowledge packet streaming
    while (inb(KEY_PORT) != 0xFA);

    set_background_green(mouse_x, mouse_y);
    // Enable IRQ12
    enable_irq(MOUSE_IRQ);
}

void mouse_handler()
{
    uint8_t packet;
    int8_t x_move;
    int8_t y_move;
    uint8_t x_sign;
    uint8_t y_sign;
    uint8_t l_button;
    uint8_t data;

    data = inb(MOUSE_PORT);

    // Is the byte available to read
    if( (data & 0x01) == 0)
    {
        send_eoi(MOUSE_IRQ);
        return;
    }
    // Is it from mouse
    data = inb(MOUSE_PORT);
    if( (data & 0x20) == 0)
    {
        send_eoi(MOUSE_IRQ);
        return;
    }


    packet = inb(KEY_PORT);
    // Discard entire packet if wrong
    if ( (packet & GET_SECOND) == GET_SECOND || ( packet & GET_FIRST ) == GET_FIRST || (packet & GET_FIFTH) == 0)
    {
        send_eoi(MOUSE_IRQ);
        return;
    }

    y_sign = (packet & GET_THIRD);
    x_sign = (packet & GET_FOURTH);
    l_button = (packet & GET_LAST);

    x_move = read_keyboard_port();
    x_move = (x_move - (x_sign << 4)); // magic number to get x_sign

    y_move = read_keyboard_port();
    y_move = (y_move - (y_sign << 3)); // magic number to get y_sign
    y_move = -y_move;                  // the y_move is different from convention

    // slow down the rate of the "cursor"
    x_move = x_move / SLOW_RATE;
    y_move = y_move / SLOW_RATE;

    if (x_move + mouse_x < 0)
        mouse_x = 0;
    else if (x_move + mouse_x >= WIDTH)
        mouse_x = WIDTH - 1;
    else
        mouse_x = mouse_x + x_move;

    if (y_move + mouse_y < 0)
        mouse_y = 0;
    else if (y_move + mouse_y >= HEIGHT)
        mouse_y = HEIGHT - 1;
    else
        mouse_y = mouse_y + y_move;

    if (mouse_x != previous_x || mouse_y != previous_y)
    {
        set_background_green(mouse_x, mouse_y);
        set_background_black(previous_x, previous_y);
    }


    previous_x = mouse_x;
    previous_y = mouse_y;

    if ( l_button == GET_LAST )
        terminal_switch(0);

    send_eoi(MOUSE_IRQ);

    return;
}
