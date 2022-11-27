#ifndef _IDT_H
#define _IDT_H

#include "x86_desc.h"


#define PIT_VEC 0x20
#define KEYBOARD_VEC  0x21
#define RTC_VEC  0x28
#define MOUSE_VEC  0x2c
#define SYS_CALL_VEC  0x80

void idt_exception_init();


extern void idt_fill(); //used to initilize idt
void idt_0();   //divide_error
void idt_1();   //debug
void idt_2();   //nmi
void idt_3();   //breakpoint
void idt_4();   //overflow
void idt_5();   //bound_range_exceeded
void idt_6();   //invalid_opcode
void idt_7();   //device_not_available
void idt_8();   //double_fault
void idt_9();   //coprocessor_segment_overrun
void idt_10();  //invalid_TSS
void idt_11();  //segment_not_present
void idt_12();  //stack_segment_fault
void idt_13();  //general_protection_fault
void idt_14();  //page_fault
void idt_15();  //spurious_interrupt_bug
void idt_16();  //coprocessor_error
void idt_17();  //alignment_check
void idt_18();  //machine_check
void idt_19();  //simd_coprocessor_error



#endif

