#define ASM 1  
#include "handlers.h"

.text
.global keyboard_linkage,rtc_linkage

# two handlers used to link the asm and c functions
keyboard_linkage:
    pushal
    call keyboard_handler
    popal
    iret
    
rtc_linkage:
    pushal
    pushfl
    call rtc_handler
    popfl
    popal
    iret
