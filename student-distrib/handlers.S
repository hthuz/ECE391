#define ASM 1  
#include "handlers.h"

.data
sys_call_table:
.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

.text
.global pit_linkage, keyboard_linkage, mouse_linkage, rtc_linkage, sys_call_linkage

.macro PUSH_TEN_PARA
    # ss,ESP,EFLAGS,CS,return adress has been pushed by processor
    push  %fs
    push  %es
    push  %ds
    pushl %eax
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
.endm

.macro POP_TEN_PARA
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %eax
    popl %ds
    popl %es
    popl %fs
.endm

pit_linkage:
    pushl $0
    pushl $8
    PUSH_TEN_PARA
    call pit_handler
    # call tackle_signal
    POP_TEN_PARA
    addl $8,%esp
    iret

keyboard_linkage:
    pushl $0
    pushl $8
    PUSH_TEN_PARA
    call keyboard_handler
    # call tackle_signal
    POP_TEN_PARA
    addl $8,%esp
    iret

mouse_linkage:
    pushal
    pushfl
    call mouse_handler
    popfl
    popal
    iret
     
rtc_linkage:
    pushl $0
    pushl $8
    PUSH_TEN_PARA
    call rtc_handler
    # call tackle_signal
    POP_TEN_PARA
    addl $8,%esp
    iret

sys_call_linkage:
  # Arguments
  cli

  # CPU has pushed five things automatically
  pushl $0
  pushl $0x80
  PUSH_TEN_PARA

  # Check system call number
  cmpl $1,%eax
  jl syscall_error
  cmpl $10,%eax
  jg syscall_error

  # Call system call function
  sti
  call *sys_call_table(,%eax,4)
  movl %eax,24(%esp)
  call tackle_signal

  cli
  POP_TEN_PARA
  addl $8,%esp
  sti

  iret

syscall_error:
  POP_TEN_PARA
  addl $8,%esp
  movl $-1, %eax
  sti
  iret

