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
void mouse_linkage(void);

extern void exp_0();
extern void exp_1();
extern void exp_2();
extern void exp_3();
extern void exp_4();
extern void exp_5();
extern void exp_6();
extern void exp_7();
extern void exp_8();
extern void exp_9();
extern void exp_10();
extern void exp_11();
extern void exp_12();
extern void exp_13();
extern void exp_14();
extern void exp_15();
extern void exp_16();
extern void exp_17();
extern void exp_18();
extern void exp_19();
#endif

#endif
