
/* terminal.c: terminal part of kernel
*/

#include "terminal.h"
#include "lib.h"
#include "keyboard.h"


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
 *   SIDE EFFECTS: none
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{

    // do nothing until enter is pressed
    while(enter_pressed == 0);

    int i;
    for(i = 0; i < nbytes; i++)
    {
        buf[i] == kb_buf[i];
    }


    return 0;
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
        printf("terminal write failure\n");
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
