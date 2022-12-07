#ifndef SIGNAL_H
#define SIGNAL_H

#ifndef ASM_SIGNAL

#define asmlinkage  __attribute__((regparm(0)))

#include "types.h"
#include "syscall.h"
#include "idt.h"
// typedef uint32_t sigset_t;
// typedef int32_t (*sighandler_t)(void);    // sighandler_t = int32_t ???


// typedef struct signal_info {
//     int32_t sigpending[5];
//     sigset_t blocked[5];
//     sigset_t pre_blocked[5];
//     sighandler_t sighand[5]; 
//     uint32_t alarm_time; 
//  } signal_info;


int32_t init_process_signal( pcb_t* the_pcb);
void init_default();
int32_t divzero_handler();
int32_t segfault_handler();
int32_t interrupt_handler();
int32_t alarm_handler();
int32_t user1_handler();

int32_t set_signal(int signum, sighandler_t action);
int32_t sys_sig_return();

int32_t send_signal( uint32_t signum );
int32_t mask_signal(int32_t signum);
int32_t unmask_signal(int32_t signum);
void restore_block();




#endif
#endif
