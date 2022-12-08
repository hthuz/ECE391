#define ASM 1  
#include "handlers.h"

.data
sys_call_table:
.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler,sigreturn_function

.text
.global pit_linkage, keyboard_linkage, mouse_linkage, rtc_linkage, sys_call_linkage

.global exp_0, exp_1, exp_2, exp_3, exp_4, exp_5, exp_6, exp_7, exp_8, exp_9, exp_10, exp_11, exp_12, exp_13, exp_14, exp_15, exp_16, exp_17, exp_18, exp_19

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
    POP_TEN_PARA
    addl $8,%esp
    iret

keyboard_linkage:
    pushl $0
    pushl $8
    PUSH_TEN_PARA
    call keyboard_handler
    call tackle_signal
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



exp_0:
         pushl $0 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_1:
         pushl $1 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_2:
         pushl $2 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_3:
         pushl $3 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_4:
         pushl $4 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_5:
         pushl $5 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_6:
         pushl $6 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_7:
         pushl $7 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
         
exp_8:
         pushl $8 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_9:
         pushl $9 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_10:
         pushl $10 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_11:
         pushl $11 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_12:
         pushl $12 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_13:
         pushl $13 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_14:
         pushl $14 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_15:
         pushl $15 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_16:
         pushl $16 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_17:
         pushl $17 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_18:
         pushl $18 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
exp_19:
         pushl $19 
         PUSH_TEN_PARA
         call exception_shower
         call tackle_signal
         POP_TEN_PARA
         addl $8,%esp
         IRET
