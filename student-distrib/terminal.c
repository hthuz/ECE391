
/* terminal.c: terminal part of kernel
*/

#include "terminal.h"
#include "lib.h"
#include "keyboard.h"


// Note that terminal_read will clear kb buffer, thus change kb_buf_length
// To test terminal read return correct number, keep a copy of kb_buf_length
int original_kb_buf_length;
/* terminal_open
 *   DESCRIPTION: get directory entry to filename
                  allocate unused file descriptor  
                  set up necessary data
 *   INPUTS: filename -- terminal filename
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if filename doesn't exist or no descriptors free
 *                  0 if success
 *   SIDE EFFECTS: initialize terminal
 */ 
int32_t terminal_open(const uint8_t* filename)
{
    return 0;
}

/* terminal_close
 *   DESCRIPTION: close fd and make it available
 *   INPUTS:  fd -- file descriptor  
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if try to close invalid fd
 *                  0 if success
 *   SIDE EFFECTS: make one fd available
 */
int32_t terminal_close(int32_t fd)
{
    // if try to close default descriptor
    if(fd == 0 || fd == 1)
        return -1;
    
    return 0;
}


/* terminal_read
 *   DESCRIPTION: copy from keyboard buffer to buf
                  only returns when enter key is pressed
 *   INPUTS:  fd -- file descriptor
              buf -- buffer  
              nbytes -- number of bytes to read
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if failure
 *                 number of bytes successfully copied if success
 *   SIDE EFFECTS: will clean keyboard buffer
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{

    // do nothing until enter is pressed
    while(enter_pressed == 0);
    cli();

    // if enter is pressed
    // copy from kb_buf until is pressed
    int i = 0; // index for copy, the final value should be kb_buf_length - 1
    int j = 0; // index for empty
    char* charbuf = buf;
    //empty buf first
    for(j = 0; j < KB_BUF_SIZE; j++)
    {
        charbuf[j] = '\0';
    }

    while(i < KB_BUF_SIZE && kb_buf[i] != '\n')
    {
        charbuf[i] = kb_buf[i];
        kb_buf[i] = '\0'; 
        i++;
    }

    // clean keyboard buffer
    // IF kb_buffer is not full, last character is /n
    // it should be cleaned
    if( i != KB_BUF_SIZE)
        kb_buf[i] = '\0';

    // used to test terminal read return correct value
    original_kb_buf_length = kb_buf_length;
    // if kb_buffer is full, treat the last /n as additional character
    if(i == KB_BUF_SIZE)
        original_kb_buf_length += 1;
    kb_buf_length = 0;
    enter_pressed = 0;
    sti();
    return i;
}


/* terminal_write
 * terminal_write
 *   DESCRIPTION: print things in buf to screen(write data to terminal)
 *   INPUTS:  fd -- file descriptor  
 *            buf -- buffer
 *            nbytes -- number of bytes to print
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if failure
 *                 number of bytes successfully written/printed if success 
 *   SIDE EFFECS: none
 */
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes)
{
    cli();
    int i;
    char* charbuf = (char*) buf;
    if( nbytes < 0 || nbytes > KB_BUF_SIZE)
    {
        sti();
        return -1;
    }

    for(i = 0; i < nbytes; i++)
    {
        putc(charbuf[i]);
    }
    sti();
    return nbytes;

}
