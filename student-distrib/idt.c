#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "handlers.h"

/*
 * idt_fill
 * DESCRIPTION: initialize interrupt descriptor and fill them into the idt
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: all the idt[] array's struct value and part of the offset
 */
void idt_fill()
{
    int i;
    // Empty all IDT
    for (i = 0; i <= NUM_VEC; i++)
    {
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1; // not sure
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;      // size is 32 bits
        idt[i].reserved0 = 0; //
        idt[i].dpl = 0;       // kernel
        idt[i].present = 0;   // firstly do not present
    }
    // Exception
    idt_exception_init();

    // reserved0 | size | reserved1 | reserved2 | reserved3 | resserved4(8bits)
    // reserved3 = 1 -- Trap Gate (enable interrupt) (doesn't affect IF flag)
    // reserved3 = 0 -- Interrupt Gate (disable interrupt) (clear IF flag and restore original IF on iret)
    // Interrupt
    idt[PIT_VEC].present = 1;
    idt[KEYBOARD_VEC].present = 1;
    idt[RTC_VEC].present = 1;

    idt[PIT_VEC].reserved3 = 0;
    idt[KEYBOARD_VEC].reserved3 = 0;
    idt[RTC_PORT].reserved3 = 0;

    SET_IDT_ENTRY(idt[PIT_VEC], pit_linkage);
    SET_IDT_ENTRY(idt[KEYBOARD_VEC], keyboard_linkage);
    SET_IDT_ENTRY(idt[RTC_VEC], rtc_linkage);

    // System Call
    idt[SYS_CALL_VEC].present = 1;
    idt[SYS_CALL_VEC].dpl = 3;
    SET_IDT_ENTRY(idt[SYS_CALL_VEC], sys_call_linkage);

}

idt_exception_init()
{
    int i;

    // Set 19 exceptions
    for (i = 0; i <= 19; i++)
        idt[i].present = 1;
    // 15 is reserved for Intel
    idt[15].present = 0; 

    SET_IDT_ENTRY(idt[0], idt_0);   // divide_error
    SET_IDT_ENTRY(idt[1], idt_1);   // debug
    SET_IDT_ENTRY(idt[2], idt_2);   // nmi
    SET_IDT_ENTRY(idt[3], idt_3);   // breakpoint
    SET_IDT_ENTRY(idt[4], idt_4);   // overflow
    SET_IDT_ENTRY(idt[5], idt_5);   // bound_range_exceeded
    SET_IDT_ENTRY(idt[6], idt_6);   // invalid_opcode
    SET_IDT_ENTRY(idt[7], idt_7);   // device_not_available
    SET_IDT_ENTRY(idt[8], idt_8);   // double_fault
    SET_IDT_ENTRY(idt[9], idt_9);   // coprocessor_segment_overrun
    SET_IDT_ENTRY(idt[10], idt_10); // invalid_TSS
    SET_IDT_ENTRY(idt[11], idt_11); // segment_not_present
    SET_IDT_ENTRY(idt[12], idt_12); // stack_segment_fault
    SET_IDT_ENTRY(idt[13], idt_13); // general_protection_fault
    SET_IDT_ENTRY(idt[14], idt_14); // page_fault
    SET_IDT_ENTRY(idt[15], idt_15); // spurious_interrupt_bug
    SET_IDT_ENTRY(idt[16], idt_16); // coprocessor_error
    SET_IDT_ENTRY(idt[17], idt_17); // alignment_check
    SET_IDT_ENTRY(idt[18], idt_18); // machine_check
    SET_IDT_ENTRY(idt[19], idt_19); // simd_coprocessor_error
}

/*
 * idt_0,idt1,idt2......idt_19
 * DESCRIPTION: exceptions handlers
 * INPUTS: none
 * OUTPUTS: prinf error message
 * RETURN VALUE: none
 * SIDE EFFECT: loop in the function
 */

void idt_0()
{
    cli();
    printf("divide_error");
    while (1)
        ;
}
void idt_1()
{
    cli();
    printf("debug error");
    while (1)
        ;
}
void idt_2()
{
    cli();
    printf("nmi error");
    while (1)
        ;
}
void idt_3()
{
    cli();
    printf("breakpoint error");
    while (1)
        ;
}
void idt_4()
{
    cli();
    printf("overflow error");
    while (1)
        ;
}
void idt_5()
{
    cli();
    printf("bound_range_exceeded error");
    while (1)
        ;
}
void idt_6()
{
    cli();
    printf("invalid_opcode error");
    while (1)
        ;
}
void idt_7()
{
    cli();
    printf("device_not_available error");
    while (1)
        ;
}
void idt_8()
{
    cli();
    printf("double_fault error");
    while (1)
        ;
}
void idt_9()
{
    cli();
    printf("coprocessor_segment_overrun error");
    while (1)
        ;
}
void idt_10()
{
    cli();
    printf("invalid_TSS error");
    while (1)
        ;
}
void idt_11()
{
    cli();
    printf("segment_not_present error");
    while (1)
        ;
}
void idt_12()
{
    cli();
    printf("stack_segment_fault error");
    while (1)
        ;
}
void idt_13()
{
    cli();
    printf("general_protection_fault error");
    while (1)
        ;
}
void idt_14()
{
    cli();
    printf("page_fault error");
    while (1)
        ;
}
void idt_15()
{
    cli();
    printf("spurious_interrupt_bug error");
    while (1)
        ;
}
void idt_16()
{
    cli();
    printf("coprocessor_error error");
    while (1)
        ;
}
void idt_17()
{
    cli();
    printf("alignment_check error");
    while (1)
        ;
}
void idt_18()
{
    cli();
    printf("machine_check error");
    while (1)
        ;
}
void idt_19()
{
    cli();
    printf("simd_coprocessor_error error");
    while (1)
        ;
}
