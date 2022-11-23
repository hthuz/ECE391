
/* terminal.c: terminal part of kernel
 */

#include "terminal.h"
#include "lib.h"
#include "syscall.h"
#include "paging.h"
#include "keyboard.h"

// Current Terminal that user is on
int32_t cur_tid;
// CUrrent Termianl that scheduling is running
int32_t running_tid;

int term_switch_flag;
int32_t term_num;

termin_t terminals[MAX_TERM_NUM];

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
        // do nothing until enter is pressed
    while (enter_pressed == 0);

    cli();
    int i;
    int j;
    char* charbuf = buf;
    termin_t* cur_term = get_terminal(cur_tid);
    if (nbytes < cur_term->kb_buf_length + 1)
        return SYSCALL_FAIL;

    // empty buf first
    for (i = 0; i <= nbytes; i++)
        charbuf[i] = '\0';

    // Copy kb_buf to dest buf
    // After loop, i becomes kb_buf_length
    for(i = 0; i < cur_term->kb_buf_length; i++)
        charbuf[i] = cur_term->kb_buf[i];

    // If kb_buf is full without /n, add one to termianl buf
    // i.e. there should always be \n at the end of terminal buf
    if(cur_term->kb_buf_length == KB_BUF_SIZE && cur_term->kb_buf[KB_BUF_SIZE - 1] != '\n')
    {
        charbuf[i] = '\n';
        i++;
    }

    // Clean kb_buf
    for(j = 0; j < cur_term->kb_buf_length; j++)
        cur_term->kb_buf[j] = '\0';
    cur_term->kb_buf_length = 0;
    enter_pressed = 0;
    sti();

    if(0 == strncmp(charbuf, "ps\n",3))
      show_task();

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

  if(cur_tid == running_tid)
  {
    for (i = 0; i < nbytes; i++)
      putc(charbuf[i]);
  }
  else
  {
    for (i = 0; i < nbytes; i++)
      terminal_putc(charbuf[i],cur_tid);
  }

  sti();
    return nbytes;
}



/*
 * terminal_init
 *   DESCRIPTION: initialize multi-terminal
 *                and invoke termianl 0 by default
 *     INPUTS: none
 *     OUTPUTS: none
 *     RETURN VALUE: none
 *     SIDE EFFECTS: none
 */
void terminal_init()
{
  int tid;
  int vid_addr;
  int i;
  for(tid = 0; tid < MAX_TERM_NUM; tid++)
  {
    terminals[tid].invoked = 0;
    // Video Memory
    vid_addr = TERM_VID_ADDR(tid);
    terminals[tid].video_mem = (char*)(TERM_VID_ADDR(tid));

    // Paging 
    p_table[PTE_INDEX(vid_addr)].base_addr = vid_addr >> 12;
    p_table[PTE_INDEX(vid_addr)].present = 1;

    // Clear Video Memory
    memset((void *)vid_addr, 0,P_4K_SIZE);

    // Screen Position
    terminals[tid].screen_x = 0;
    terminals[tid].screen_y = 0;

    // Keyboard buffer
    for(i = 0; i < KB_BUF_SIZE; i++)
      terminals[tid].kb_buf[i] = '\0';
    terminals[tid].kb_buf_length = 0;

    for(i = 0; i < MAX_TASK_NUM; i++)
      terminals[tid].pid_list[i] = NO_PID;
    terminals[tid].pid_num = 0;
    terminals[tid].pid = NO_PID;
  }    


  // Terminal 0 is invoked 
  terminals[0].invoked = 1;


  // Initialize global variables
  cur_tid = 0;
  running_tid = 0;
  term_num = 1;
  term_switch_flag = 0;

}

/*
 * get_terminal
 *   DESCRIPTION: Get pointer to terminal struct 
 *                based on terminal id
 *   INPUTS: tid -- terminal id(0,1,2)
 *   OUTPUTS: none
 *   RETURN VALUE: The corresponding termianl struct
 *   SIDE EFFECTS: none
 */
termin_t* get_terminal(int32_t tid)
{
  return &terminals[tid];
}


/*
 * terminal_init
 *   DESCRIPTION: When Alt+ Funckey is pressed,
 *                Do termianl switch staff
 *   INPUTS: new_tid -- terminal id to switch into
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void terminal_switch(int32_t new_tid)
{

  // If Switch to the same terminal, do nothing
  if(cur_tid == new_tid)
    return;
  
  pcb_t* cur_pcb = get_pcb(cur_pid);
  termin_t* cur_term = get_terminal(cur_tid);
  termin_t* new_term = get_terminal(new_tid);
  // Save current used terminal video memory
  memcpy((void*)cur_term->video_mem,(const void*) VID_MEM_START, P_4K_SIZE);
  
  // Set new terminal video memory
  memcpy((void*) VID_MEM_START, (const void*)new_term->video_mem, P_4K_SIZE);

  // Save screen positoin
  cur_term->screen_x = screen_x;
  cur_term->screen_y = screen_y;

  // Set new screen position
  screen_x = new_term->screen_x;
  screen_y = new_term->screen_y;
  update_cursor(screen_x,screen_y);

  cur_tid = new_tid;

  // Start new shell if it's not invoked
  if(new_term->invoked == 0)
  {
    new_term->invoked = 1;
    term_num++;
    term_switch_flag = 1;

    asm volatile(
      "movl %%ebp, %0;"
      "movl %%esp, %1"
      :"=r"(cur_pcb->saved_ebp), "=r"(cur_pcb->saved_esp)
    );
    // cur_pid = ROOT_PID; // Not Necessary
    printf("TERMINAL #%d\n",cur_tid);
    execute((uint8_t *)"shell");
  }


}





