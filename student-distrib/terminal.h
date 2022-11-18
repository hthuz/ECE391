/* termial.h - Defines used in interactions with terminal
*/


#ifndef TERMINAL_H
#define TERMINAL_H

#include "lib.h"
#include "paging.h"

// Multiple Terminal Macros
#define MAX_TERM_NUM 3
// Terminal 1: 800KB - 0xC8000
// Terminal 2: 900KB - 0xE1000
// Termiaal 3: 1000KB  - 0xFA000 
#define TERM_VID_ADDR(tid) ((tid) * 25 * P_4K_SIZE + 175 * P_4K_SIZE)

extern int32_t cur_tid;
// terminal driver components
int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void * buf, int32_t nbytes);

void terminal_switch(int32_t new_tid);
void terminal_paging_init();

#endif /*TERMINAL_H*/

