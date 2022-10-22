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


unsigned char capital_scancode[80] = 
{
	0,    0, '!', '@', '#', '$', '%', '^', '&', '*', 
  '(',  ')', '_', '+','\b',   0, 'Q', 'W', 'E', 'R', 
  'T',  'Y', 'U', 'I', 'O', 'P', '{', '}','\n',  0 , 
  'A',  'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',    
 '\"',  '~',  0 , '|', 'Z', 'X', 'C', 'V', 'B', 'N', 
  'M',  '<', '>', '?',   0,  0 ,  0 , ' ',  0 ,  0 
};
      


#define CTRL 0x1D   //for right ctrl, generate two interrupts, first is 0xE0 and second is 0x1D
#define REL_CTRL 0x9D

#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define REL_LEFT_SHIFT 0xAA
#define REL_RIGHT_SHIFT 0xB6

#define ALT 0x38 //for right alt generate two interrupts, first is 0xE0 and second is 0x38
#define REL_ALT 0xB8

#define CAPSLOCK 0x3A
#define EXT_BYTE 0xE0   

int ctrl_pressed = 0;
int shift_pressed = 0;
int alt_pressed = 0;
int capslock_on = 0;  // Assume capslock is off at the beginning

// The size of keyboard buffer is at most 128 bytes
unsigned char kb_buf[KB_BUF_SIZE] = {'\0'};
int kb_buf_length = 0;		//Number of characters in kb buffer ( 0 - 128)
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
	int i;

	unsigned char c = inb(KEY_PORT);
	// printf("%x ",c);
	// if get EXT_BYTE, read scancode
	if ( c == EXT_BYTE)
	{
		c = inb(KEY_PORT);
	}

	// if get CTRL
	if (c == CTRL)
		ctrl_pressed = 1;
	if (c == REL_CTRL)
		ctrl_pressed = 0;
	// if get ALT
	if (c == ALT)
		alt_pressed = 1;
	if (c == REL_ALT)
		alt_pressed = 0;
	// if get SHIFT
	if (c == LEFT_SHIFT || c == RIGHT_SHIFT)
		shift_pressed = 1;
	if (c == REL_LEFT_SHIFT || c == REL_RIGHT_SHIFT)
		shift_pressed = 0;

	// CTRL + L will clean the screen
	// 0x26: scancode for L
	if (ctrl_pressed == 1 && c == 0x26)
	{
		clear();
		memset(kb_buf,'\0',KB_BUF_SIZE);
		kb_buf_length = 0;
		send_eoi(KEY_IRQ);
		return;
	}

	// print the character
	if (c>=80) {
		send_eoi(KEY_IRQ);	// if it is outside the scancode table, ignore it 
		return;
	}

	// if shift is pressed, use capital result  
	if(shift_pressed == 1)
	{
		result = capital_scancode[c];
	}
	else
		result = scancode[c];

	//Case1: If get backspace
	if (result == '\b')
	{
		// only delete if buffer length is not 0
		if (kb_buf_length != 0)
		{
			// if it's tab on buffer,delete four times
			if(kb_buf[kb_buf_length - 1] == '\t')
			{
				for (i = 0; i < 4; i++)
					putc(result);
			}
			//else, only delete one time
			else
			{
				putc(result);
			}
			kb_buf[kb_buf_length - 1] = '\0';
			kb_buf_length--;
		}	
		send_eoi(KEY_IRQ);
		return;	
	}

	// Case2: If get other characters
	// If kb buffer doesn't overflow, put the result into buffer  
	if(kb_buf_length != KB_BUF_SIZE  && result != 0)
	{
		// if this line reaches end, automatic add new line
		// but shouldn't be added to buffer as this may affect the complence of command
		if(screen_x == NUM_COLS - 1)
			putc('\n');
		kb_buf[kb_buf_length] = result;
		kb_buf_length++;
		putc(result);
		send_eoi(KEY_IRQ);
		return;	
	}
	// if all these conditions are not met, still need to send EOI
	send_eoi(KEY_IRQ);
	return;	
}

