#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

/* Array for the characters without shift or CAPS */
unsigned char scancode[80] =
{
	0,    0, '1', '2', '3', '4', '5', '6', '7', '8', 
  '9',  '0', '-', '=',  0 ,  0 , 'q', 'w', 'e', 'r', 
  't',  'y', 'u', 'i', 'o', 'p', '[', ']','\n',  0 , 
  'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    
   0 ,  '`',  0 ,'\\', 'z', 'x', 'c', 'v', 'b', 'n', 
  'm',  ',', '.', '/',   0,  0 ,  0 , ' ',  0 ,  0 
};


/* 
 * initialize_keyboard
 * DESCRIPTION: enable the keyboard interrupt IRQ1
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: enable IRQ1
 */
void initialize_keyboard() {
	enable_irq(KEY_IRQ);	// enable keyboard_irq
}


/* 
 * keyboard_c_handler
 * DESCRIPTION: when you have a keyboard interrupt, this function is called and use inb to 
 * 				get your input to print it if it is in the scancode table
 * INPUTS: none
 * OUTPUTS: output the keyboard value
 * RETURN VALUE: none
 * SIDE EFFECT: after that, it will send EOI to tell the cpu it has done
 */

void keyboard_c_handler()
{
	unsigned char result;
	unsigned char c = inb(KEY_PORT);
	if (c>=80) {
		send_eoi(KEY_IRQ);	// if it is outside the scancode table, ignore it 
		return;
	}
	result = scancode[c];
	// if result=0, you will print a blank
	putc(result);				//else print it
	send_eoi(KEY_IRQ);		// send the message of ending interrupt
}

