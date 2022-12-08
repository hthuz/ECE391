
#define SIGNAL_ASM  1

#include "signal.h"
.globl user_mode_handler, sigreturn_function


sys_sigreturn:
        movl $10,%eax
        INT  $0x80
return_address_for_sigreturn:
        .long 0

process_kernel_ebp:
        .long 0
process_kernel_esp:
        .long 0
user_ebp:
        .long 0
user_esp:
        .long 0

bootimg_stack_hw_ptr:
        .long 0
handler_function_ptr_value:
        .long 0
signum_value:
        .long 0

bootimg_stack_hw_ptr2:
        .long 0

user_mode_handler:

    # now in process kernel stack
    pushl %ebp
    movl  %esp, %ebp

    # store the register
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %edx
    pushl %ecx

    # store process kernel stack esp and ebp
    movl %esp, process_kernel_esp
    movl %ebp, process_kernel_ebp

    # store three parameters in process kernel stack
    movl 8(%ebp), %ebx 
    movl %ebx, signum_value
    movl 12(%ebp), %ebx 
    movl %ebx, handler_function_ptr_value
    movl 16(%ebp), %ebx 
    movl %ebx, bootimg_stack_hw_ptr 


    # store the user stack esp and ebp
    movl 60(%ebx),%edi
    movl %edi, user_esp
    movl 20(%ebx),%esi
    movl %esi, user_ebp

    # switch to user stack
    movl %edi, %esp
    movl %esi, %ebp


    # now in user stack
    # do not need to push ebp 

    # go to push sys_sigreturn 
    movl sys_sigreturn, %ebx
    subl $7,%esp
    movl %ebx, 0(%esp)
    movl sys_sigreturn +3, %ebx
    movl %ebx, 3(%esp)

    # prepare the user stack sys_sigreturn address
    movl %esp, return_address_for_sigreturn

    # align 4
    subl $1, %esp

    # copy the bootimg_stack_hw into user stack

    movl bootimg_stack_hw_ptr, %ebx
          pushl 64(%ebx)  # xss
          pushl 60(%ebx)  # esp
          pushl 56(%ebx)  # eflags
          pushl 52(%ebx)  # xcs
          pushl 48(%ebx)  # return address
          pushl 44(%ebx)  # error_code/dummy
          pushl 40(%ebx)  # IRQ
          pushl 36(%ebx)  # xfs
          pushl 32(%ebx)  # xes
          pushl 28(%ebx)  # ds
          pushl 24(%ebx)  # eax
          pushl 20(%ebx)  # ebp
          pushl 16(%ebx)  # edi
          pushl 12(%ebx)  # esi
          pushl 8(%ebx)   # edx
          pushl 4(%ebx)   # ecx
          pushl 0(%ebx)   # ebx


    # to to push signal number and push return address for sigreturn
    pushl signum_value
    pushl return_address_for_sigreturn

    # update the eip and esp in bootimg kernel stack
    # so that iret will excute handler fucntion
    movl %esp, 60(%ebx)
    
    movl handler_function_ptr_value, %esi
    movl %esi, 48(%ebx)

    # back to process kernel stack , 
    # return to bootimg kernel stack

    movl process_kernel_esp, %esp
    movl process_kernel_ebp, %ebp
    
    popl %ecx
    popl %edx
    popl %edi
    popl %esi
    popl %ebx

    leave
    ret

sigreturn_function:

    # we cannot push any register here
    # we cannot use edx to store the pointer
    movl %esp,%esi
    addl $4, %esi
    movl %esi, bootimg_stack_hw_ptr2

    movl 64(%esp),%edi
    addl $4, %edi


    movl $0,%ebx
loop:
    movl (%edi),%ecx
    movl %ecx,(%esi)
    addl $4, %esi
    addl $4, %edi
    addl $1, %ebx
    cmpl $17, %ebx
    jne loop

    call restore_block

    # the oldest eax
    movl bootimg_stack_hw_ptr2,%esi
    movl  24(%esi),%eax

    ret