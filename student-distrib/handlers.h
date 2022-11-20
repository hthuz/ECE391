#ifndef _MYHAND_H
#define _MYHAND_H

#ifndef ASM

#include "keyboard.h"
#include "rtc.h"
#include "syscall.h"

/*keyboard wrapper for assembly linkage*/
void pit_linkage(void);
void keyboard_linkage(void);
void rtc_linkage(void);
void sys_call_linkage(void);

#endif

#endif
