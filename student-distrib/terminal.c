
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
int32_t terminal_open(const uint8_t *filename)
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
    return 0;
}

/* terminal_read
 *   DESCRIPTION: copy from keyboard buffer to buf
                  only returns when enter key is pressed
 *   INPUTS:  fd -- file descriptor
              buf -- buffer to load keyboard buffer
              nbytes -- maximum length of buf - 1
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if failure
 *                 number of bytes successfully copied if success
 *   SIDE EFFECTS: will clean keyboard buffer
 */
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    if (buf == NULL)
        return -1;
    if (nbytes < kb_buf_length + 1)
        return -1;
    // do nothing until enter is pressed
    while (enter_pressed == 0);

    cli();
    int i;
    int j;
    char *charbuf = buf;

    // empty buf first
    for (i = 0; i <= nbytes; i++)
        charbuf[i] = '\0';

    // Copy kb_buf to dest buf
    // After loop, i becomes kb_buf_length
    for(i = 0; i < kb_buf_length; i++)
    {
        charbuf[i] = kb_buf[i];
    }

    // If kb_buf is full without /n, add one to termianl buf
    // i.e. there should always be \n at the end of terminal buf
    if(kb_buf_length == KB_BUF_SIZE && kb_buf[KB_BUF_SIZE - 1] != '\n')
    {
        charbuf[i] = '\n';
        i++;
    }

    // Clean kb_buf
    for(j = 0; j < kb_buf_length; j++)
        kb_buf[j] = '\0';
    kb_buf_length = 0;
    enter_pressed = 0;
    sti();
    return i ;
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
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes)
{
    if (nbytes < 0)
      return -1;
    if (buf == NULL)
      return -1;

    cli();
    int i;
    char *charbuf = (char *)buf;

    for (i = 0; i < nbytes; i++)
    {
        putc(charbuf[i]);
    }
    sti();
    return nbytes;
}





