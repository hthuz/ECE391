/* termial.h - Defines used in interactions with terminal
*/


#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "paging.h"
#include "keyboard.h"

#define MAX_TERM_NUM 3
// Don't know why but the following example fails
// Fail to copy memory
// Terminal 0: 800KB - 0xC8000
// Terminal 1: 900KB - 0xE1000
// Termiaal 2: 1000KB  - 0xFA000 
// #define TERM_VID_ADDR(tid) ((tid) * 25 * P_4K_SIZE + 200 * P_4K_SIZE)
#define TERM_VID_ADDR(tid) (VID_MEM_START + (tid + 2) * P_4K_SIZE)

extern int32_t cur_tid;

struct termin_t
{
  int invoked;
  char* video_mem;
  int screen_x;
  int screen_y;
  unsigned char kb_buf[KB_BUF_SIZE];
  int kb_buf_length;
  int pid;
  

};

typedef struct termin_t termin_t;







// terminal driver components
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);

// multi-terminal 
void terminal_init();
termin_t* get_terminal(int32_t tid);
void terminal_switch(int32_t new_tid);

#endif /*TERMINAL_H*/

