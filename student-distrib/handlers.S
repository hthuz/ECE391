#define ASM 1  
#include "handlers.h"

.data
sys_call_table:
.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.text
.global pit_linkage, keyboard_linkage, mouse_linkage, rtc_linkage, sys_call_linkage

pit_linkage:
    pushal 
    pushfl
    call pit_handler
    popfl
    popal
    iret

keyboard_linkage:
    pushal
    pushfl
    call keyboard_handler
    popfl
    popal
    iret

mouse_linkage:
    pushal
    pushfl
    call mouse_handler
    popfl
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
  # Arguments
  cli
  pushl %ebp
  pushl %edi
  pushl %esi
  pushl %edx
  pushl %ecx
  pushl %ebx

  # Check system call number
  cmpl $1,%eax
  jl syscall_error
  cmpl $10,%eax
  jg syscall_error

  # Call system call function
  sti
  call *sys_call_table(,%eax,4)
  cli
  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
  popl %ebp
  sti

  iret

syscall_error:
  movl $-1, %eax
  popl %ebx
  popl %ecx
  popl %edx
  popl %esi
  popl %edi
  popl %ebp
  sti
  iret

