#ifndef _MYHAND_H
#define _MYHAND_H

#ifndef ASM

#include "keyboard.h"
#include "rtc.h"

/*keyboard wrapper for assembly linkage*/
void keyboard_a_handler(void);
void rtc_a_handler(void);

#endif

#endif
