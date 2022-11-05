#define ASM 1  
#include "handlers.h"

.data
sys_call_table:
.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.text
.global keyboard_linkage,rtc_linkage, sys_call_linkage

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

sys_call_linkage:
  pushal
  # Check system call number
  cmpl $1,%eax
  jl syscall_error
  cmpl $10,%eax
  jg syscall_error

  # Call system call function
  call *sys_call_table(,%eax,4)
  popal
  iret

syscall_error:
  movl $-1, %eax
  popal
  iret

