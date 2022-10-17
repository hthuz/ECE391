#define ASM 1  
#include "myhand.h"

.text
.global keyboard_a_handler,rtc_a_handler

keyboard_a_handler:
    pushal
    call keyboard_c_handler
    popal
    iret
    
rtc_a_handler:
    pushal
    pushfl
    call rtc_interrupt
    popfl
    popal
    iret
