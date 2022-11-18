
/* terminal.c: terminal part of kernel
 */

#include "terminal.h"
#include "lib.h"
#include "keyboard.h"
#include "syscall.h"
#include "paging.h"


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
        return SYSCALL_FAIL;
    if (nbytes < kb_buf_length + 1)
        return SYSCALL_FAIL;
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
      return SYSCALL_FAIL;
    if (buf == NULL)
      return SYSCALL_FAIL;

    cli();
    int i;
    char *charbuf = (char *)buf;

    for (i = 0; i < nbytes; i++)
        putc(charbuf[i]);
    sti();
    return nbytes;
}


void terminal_switch(int32_t new_tid)
{
  int new_term_vid_addr = TERM_VID_ADDR(new_tid);
  int cur_term_vid_addr = TERM_VID_ADDR(cur_tid);
  
  // Save current used terminal video memory
  memcpy((void*)cur_term_vid_addr,(const void*) VID_MEM_START, P_4K_SIZE);
  
  // Set new terminal video memory
  memcpy((void*) VID_MEM_START, (const void*)new_term_vid_addr, P_4K_SIZE);

  cur_tid = new_tid;
  printf("TERMINAL #%d\n",cur_tid);
  execute((uint8_t *)"shell");
}


void terminal_paging_init()
{
  int vid_addr;
  int tid;

  // For each terminal
  for(tid = 1; tid <= MAX_TERM_NUM; tid++ )
  {
    vid_addr = TERM_VID_ADDR(tid);
    p_table[PTE_INDEX(vid_addr)].base_addr = vid_addr >> 12;
    p_table[PTE_INDEX(vid_addr)].present = 1;
  }
}




