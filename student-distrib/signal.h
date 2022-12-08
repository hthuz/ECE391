#ifndef SIGNAL_H
#define SIGNAL_H

#ifndef SIGNAL_ASM

#include "types.h"
#include "syscall.h"
#include "idt.h"

#define MAX_SIGNUM 4
extern int interrupt_shell_flag[3];
extern sighandler_t default_handle[5];


void init_default();
int32_t divzero_handler();
int32_t segfault_handler();
int32_t interrupt_handler();
int32_t alarm_handler();
int32_t user1_handler();


int32_t init_process_signal( pcb_t* the_pcb);
int32_t set_signal(int signum, sighandler_t action);
int32_t send_signal( uint32_t signum );
void tackle_signal(switch_para hard_context);
void restore_block();

extern void user_mode_handler(int32_t signum,sighandler_t hand_func,switch_para* hard_context);


#endif
#endif
