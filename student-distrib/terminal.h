/* termial.h - Defines used in interactions with terminal
*/


#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "paging.h"
#include "keyboard.h"
#include "syscall.h"

#define MAX_TERM_NUM 3
#define TERM_VID_ADDR(tid) (VID_MEM_START + (tid + 2) * P_4K_SIZE)

struct termin_t
{
  int invoked;
  int32_t pid;      // Current task that runs on this terminal
  char* video_mem;
  int screen_x;
  int screen_y;
  unsigned char kb_buf[KB_BUF_SIZE];
  int kb_buf_length;
  int enter_pressed;
  int rtc_counter;
  int rtc_freq;
  uint32_t saved_ebp;
  uint32_t saved_esp;   // Used for scheduling

};

typedef struct termin_t termin_t;


extern int32_t cur_tid;
extern int32_t running_tid;

extern int term_switch_flag;
extern int32_t term_num;
extern termin_t terminals[MAX_TERM_NUM];


// Terminal Driver Functions
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);

// Multi-terminal 
void terminal_init();
termin_t* get_terminal(int32_t tid);
void terminal_switch(int32_t new_tid);

#endif /*TERMINAL_H*/

