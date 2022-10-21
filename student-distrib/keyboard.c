#include "keyboard.h"
#include "lib.h"
#include "i8259.h"

/* Array for the characters without shift or CAPS */
// Refer to https://wiki.osdev.org/Keyboard ScanCode set 1
unsigned char scancode[80] =
{
	0,    0, '1', '2', '3', '4', '5', '6', '7', '8', 
  '9',  '0', '-', '=','\b','\t', 'q', 'w', 'e', 'r', 
  't',  'y', 'u', 'i', 'o', 'p', '[', ']','\n',  0 , 
  'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',    
'\'' ,  '`',  0 ,'\\', 'z', 'x', 'c', 'v', 'b', 'n', 
  'm',  ',', '.', '/',   0,  0 ,  0 , ' ',  0 ,  0 
};

#define left_ctrl 0x1D
#define right_ctrl 0x1D   //generate two interrupts, first is 0xE0 and second is 0x1D
#define left_shift 0x2A
#define right_shift 0x36
#define left_alt 0x38
#define right_alt 0x38   // generate two interrupts, first is 0xE0 and second is 0x38

#define capslock 0x3A


// The size of keyboard buffer is at most 128 bytes
unsigned char kb_buf[KB_BUF_SIZE] = {'\0'};
int kb_buf_index = 0;
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

	// If kb buffer doesn't overflow
	// Put the result into buffer  
	if(kb_buf_index != KB_BUF_SIZE - 1 && result != 0)
	{
		// if not backspace, add it to buffer
		if(result != '\b')
		{
			kb_buf[kb_buf_index] = result;
			kb_buf_index++;
		}
		// else, remove one character from buffer
		else
		{
			if (kb_buf_index != 0)
			{
				kb_buf_index--;
				kb_buf[kb_buf_index] = '\0';
			}
		}

	}

	// if result=0, you will print a blank
	putc(result);				//else print it
	send_eoi(KEY_IRQ);		// send the message of ending interrupt
}

