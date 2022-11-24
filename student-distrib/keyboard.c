
#include "keyboard.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"

/* Array for the characters without shift or CAPS */
// Refer to https://wiki.osdev.org/Keyboard ScanCode set 1
unsigned char scancode[0x3E] =
	{
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8',
		'9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r',
		't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
		'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
		'\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n',
		'm', ',', '.', '/', 0, 0, 0, ' ', 0, 0, 0, 0};

unsigned char capital_scancode[0x3E] =
	{
		0, 0, '!', '@', '#', '$', '%', '^', '&', '*',
		'(', ')', '_', '+', '\b', 0, 'Q', 'W', 'E', 'R',
		'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
		'\"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N',
		'M', '<', '>', '?', 0, 0, 0, ' ', 0, 0, 0, 0};

#define CTRL 0x1D // for right ctrl, generate two interrupts, first is 0xE0 and second is 0x1D
#define REL_CTRL 0x9D

#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define REL_LEFT_SHIFT 0xAA
#define REL_RIGHT_SHIFT 0xB6

#define ALT 0x38 // for right alt generate two interrupts, first is 0xE0 and second is 0x38
#define REL_ALT 0xB8

#define CAPSLOCK 0x3A
#define EXT_BYTE 0xE0
#define CAPS_OFFSET 0x20 // 0x20 offset in ASCII between lower case and upper case letters

#define FUNC1 0x3B
#define FUNC2 0x3C
#define FUNC3 0x3D

// Initialized in terminal_init function
int ctrl_pressed = 0;
int shift_pressed = 0;
int alt_pressed = 0;
int capslock_on = 0; // Assume capslock is off at the beginning
int enter_pressed = 0;

// The size of keyboard buffer is at most 128 bytes
/*
 * initialize_keyboard
 * DESCRIPTION: enable the keyboard interrupt IRQ1
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: enable IRQ1
 */
void keyboard_init()
{
	enable_irq(KEY_IRQ); // enable keyboard_irq
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

void keyboard_handler()
{
	unsigned char result;
  termin_t* cur_term = get_terminal(cur_tid);

	unsigned char c = inb(KEY_PORT);
	// if get EXT_BYTE, read scancode
	if (c == EXT_BYTE)
		c = inb(KEY_PORT);

	// Condition if get CTRL, ALT, SHIFT, CAPS_LOCK
	if (c == CTRL)
		ctrl_pressed = 1;
	if (c == REL_CTRL)
		ctrl_pressed = 0;

	if (c == ALT)
		alt_pressed = 1;
	if (c == REL_ALT)
		alt_pressed = 0;

	if (c == LEFT_SHIFT || c == RIGHT_SHIFT)
		shift_pressed = 1;
	if (c == REL_LEFT_SHIFT || c == REL_RIGHT_SHIFT)
		shift_pressed = 0;

	if (c == CAPSLOCK && capslock_on == 0)
	{
		capslock_on = 1;
		// end of interrupt as it shouldn't influence next if condition
		send_eoi(KEY_IRQ);
		return;
	}
	if (c == CAPSLOCK && capslock_on == 1)
		capslock_on = 0;

	// CTRL + L will clean the screen
	// 0x26: scancode for L
	if (ctrl_pressed == 1 && c == 0x26)
	{
    handle_clear_screen();
		send_eoi(KEY_IRQ);
		return;
	}

  // ALT + Function-key will switch terminal
  if (alt_pressed == 1 && c == FUNC1)
  {
    send_eoi(KEY_IRQ);
    terminal_switch(0);
    return;
  }
  if (alt_pressed == 1 && c == FUNC2)
  {
    send_eoi(KEY_IRQ);
    terminal_switch(1);
    return;
  }
  if (alt_pressed == 1 && c == FUNC3)
  {
    send_eoi(KEY_IRQ);
    terminal_switch(2);
    return;
  }


	// if it is outside the scancode table, do not translate
	if (c >= 0x3E)
	{
		send_eoi(KEY_IRQ); 
		return;
	}

  result = translate_scancode(c);
	// If get backspace
	if (result == '\b')
	{
    handle_backspace();
	  send_eoi(KEY_IRQ);
		return;
	}

	// If get other characters
	// If kb buffer doesn't overflow, put the result into buffer
	if (cur_term->kb_buf_length != KB_BUF_SIZE && result != 0)
	{
		cur_term->kb_buf[cur_term->kb_buf_length] = result;
	  cur_term->kb_buf_length++;
		putc(result);
    if (result == '\n')
        cur_term->enter_pressed = 1;
    send_eoi(KEY_IRQ);
    return;
	}

  // If kb_buf is full but still get enter
  if (cur_term->kb_buf_length == KB_BUF_SIZE && result == '\n')
  {
    putc(result);
    cur_term->enter_pressed = 1;
  }
	send_eoi(KEY_IRQ);
	return;
}

/* 
 * handle_clear_screen
 *   DESCRIPTION: clear the screen and clean kb_buf
 *   INPUT: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 */
void handle_clear_screen()
{
  	termin_t* cur_term = get_terminal(cur_tid);
	clear();
	update_cursor(0, 0);
	memset(cur_term->kb_buf, '\0', KB_BUF_SIZE);
	cur_term->kb_buf_length = 0;
  return;
}

/*
 * translate_scancode
 *   DESCRIPTION: translate scancode to alphanumeric character
 *   INPUT:  c -- scancode
 *   OUTPUT: none
 *   RETURN VALUE: corresponding alphanumeric character
 */
unsigned char translate_scancode(unsigned char c)
{
  unsigned char result;

  // Combinations of shift and caps conditions
	// shift not pressed, caps lock off
	if (shift_pressed == 0 && capslock_on == 0)
		result = scancode[c];

	// shift not pressed, caps lock on
	if (shift_pressed == 0 && capslock_on == 1)
	{
		result = scancode[c];
		if (is_alphabet(c))
			result = result - CAPS_OFFSET;
	}

	// shift pressed, caps lock off
	if (shift_pressed == 1 && capslock_on == 0)
		result = capital_scancode[c];

	// shift pressed, caps lock on
	if (shift_pressed == 1 && capslock_on == 1)
	{
		result = capital_scancode[c];
		if (is_alphabet(c))
			result = result + CAPS_OFFSET;
	}

  return result;
}

/*
 * handle_backspace
 *   DESCRIPTION: handler functino when backspace is pressed
 *   INPUT: none
 *   OUTPUT: none
 *   RETURN VALUE: none
 */
void handle_backspace()
{
  int i;
  termin_t* cur_term = get_terminal(cur_tid);
  unsigned char backspace = '\b';
	// only delete if buffer length is not 0
	if (cur_term->kb_buf_length != 0)
	{
		// if it's tab on buffer,delete four times
		if (cur_term->kb_buf[cur_term->kb_buf_length - 1] == '\t')
		{
			for (i = 0; i < 4; i++)
				putc(backspace);
		}
		// else, only delete one time
		else
			putc(backspace);

		cur_term->kb_buf[cur_term->kb_buf_length - 1] = '\0';
		cur_term->kb_buf_length--;
	}
  return;
}
/* is_alphabet
 *   DESCRIPTION: given a scancode from keyboard, determine if it's alphabet,
 *   			  used to help differentiate different cases of shift and caps lock
 *   INPUTS: scancode from keyboard
 *   OUTPUTS: none
 *   RETURN VALUE: 1 if the key pressed is alphabet
 *  			   0 otherwise
 *   SIDE EFFECTS: none
 *
 */
int is_alphabet(unsigned char scancode)
{
	// the following numbers are scancode for alphabets
	// used to determine if it's alphabet
	if ((scancode >= 0x10 && scancode <= 0x19) ||
		(scancode >= 0x1E && scancode <= 0x26) ||
		(scancode >= 0x2C && scancode <= 0x32))
		return 1;
	else
		return 0;
}



/* scroll_one_line
 *   DESCRIPTION: when the screen is full, screen by one line
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 if scroll succeed
 * 				   -1 if fail
 *   SIDE EFFECT: Will change the content of video memory
 */
void scroll_one_line()
{
	char *dest_mem;

	// move memory starting frow second row to first row
	dest_mem = memmove(video_mem, (char *)(VIDEO + (NUM_COLS << 1)), VIDEO_SIZE - (NUM_COLS << 1));

	// set video memory of last row to empty
	// TODO: probabilty keyboard buffer need to be cleaned as well?
	int32_t i;
	for (i = 0; i < NUM_COLS; i++)
	{
		*(uint8_t *)(video_mem + VIDEO_SIZE - (NUM_COLS << 1) + (i << 1)) = ' ';
		*(uint8_t *)(video_mem + VIDEO_SIZE - (NUM_COLS << 1) + (i << 1) + 1) = ATTRIB;
	}
	// reset position on screen
	screen_x = 0;
	screen_y--; // reset screen_y to NUM_ROWS - 1 (24)
	// update_cursor(screen_x,screen_y);

	return;
}

void terminal_scroll_one_line(int32_t tid)
{
  termin_t* term = get_terminal(tid);
	char *dest_mem;

	// move memory starting frow second row to first row
	dest_mem = memmove(term->video_mem, (char *)(TERM_VID_ADDR(tid) + (NUM_COLS << 1)), VIDEO_SIZE - (NUM_COLS << 1));

	// set video memory of last row to empty
	// TODO: probabilty keyboard buffer need to be cleaned as well?
	int32_t i;
	for (i = 0; i < NUM_COLS; i++)
	{
		*(uint8_t *)(term->video_mem + VIDEO_SIZE - (NUM_COLS << 1) + (i << 1)) = ' ';
		*(uint8_t *)(term->video_mem + VIDEO_SIZE - (NUM_COLS << 1) + (i << 1) + 1) = ATTRIB;
	}
	// reset position on screen
	term->screen_x = 0;
	term->screen_y--; // reset screen_y to NUM_ROWS - 1 (24)
	// update_cursor(screen_x,screen_y);

	return;
}

/* cursor_init
 *   DESCRIPTION: cursor initialize
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: side start and end scanline for this os
 */
void cursor_init()
{
	enable_cursor(0, NUM_ROWS - 1);
	return;
}

/* enable_cursor
 *   DESCRIPTION: enable cursor and set start and end scanlines
 *   INPUTS: cursor_start -- start scanline of cursor
 *    	     cursor_end -- end scanline of cursor
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will enable cursor
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x0A, CURSOR_PORT);
	outb((inb(CURSOR_DATA) & 0xC0) | cursor_start, CURSOR_DATA);

	outb(0x0B, CURSOR_PORT);
	outb((inb(CURSOR_DATA) & 0xE0) | cursor_end, 0x3D5);

	update_cursor(0, 0);
}

/* disable_cursor
 *   DESCRIPTION: disable cursor
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will disable cursor
 */
void disable_cursor()
{
	outb(0x0A, CURSOR_PORT);
	outb(0x20, CURSOR_DATA);
}

/* update_cursor
 *   DESCRIPTION: update cursor's position
 *   INPUTS: x -- x position of cursor
 *   		 y -- y position of cursor
 *   OUTPTUS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change cursor's position
 */

void update_cursor(int x, int y)
{
	uint16_t pos = y * NUM_COLS + x;

	// pass lower 8 bits of position
	outb(0x0F, CURSOR_PORT);
	outb((uint8_t)(pos & 0xFF), CURSOR_DATA);
	// pass upper 8 bits of position
	outb(0x0E, CURSOR_PORT);
	outb((uint8_t)((pos >> 8) & 0xFF), CURSOR_DATA);
}
