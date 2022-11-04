#ifndef _MYHAND_H
#define _MYHAND_H

#ifndef ASM

#include "keyboard.h"
#include "rtc.h"

/*keyboard wrapper for assembly linkage*/
void keyboard_linkage(void);
void rtc_linkage(void);

#endif

#endif
