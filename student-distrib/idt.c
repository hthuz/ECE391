#include "idt.h"
#include "lib.h"
#include "x86_desc.h"
#include "myhand.h"


/* 
 * idt_fill
 * DESCRIPTION: initialize interrupt descriptor and fill them into the idt
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: all the idt[] array's struct value and part of the offset
 */
void
idt_fill(){
    int i;
    // all set present=0, reserved3=
    for (i=0;i<=NUM_VEC;i++){
        idt[i].seg_selector=KERNEL_CS;
        idt[i].reserved4 =0;
        idt[i].reserved3 =0;    // not sure
        idt[i].reserved2 =1;
        idt[i].reserved1 =1;    
        idt[i].size      =1;    // size is 32 bits
        idt[i].reserved0 =0;    // 
        idt[i].dpl       =0;    // kernel
        idt[i].present   =0;    //firstly do not present
    }
    // exceptions set
    for (i=0;i<=19;i++){    // use first 19 exceptions except 15
        idt[i].present=1;
    }
    idt[15].present=0;      // magic number 15 because we don't use that
    idt[keyboard_port].present=1;      
    idt[rtc_port].present=1;
    idt[systemcall_port].present=1;
    idt[keyboard_port].reserved3=1;
    idt[rtc_port].reserved3=1;
    idt[systemcall_port].dpl=3;
    SET_IDT_ENTRY(idt[0],idt_0);    //divide_error
    SET_IDT_ENTRY(idt[1],idt_1);    //debug
    SET_IDT_ENTRY(idt[2],idt_2);    //nmi
    SET_IDT_ENTRY(idt[3],idt_3);    //breakpoint
    SET_IDT_ENTRY(idt[4],idt_4);    //overflow
    SET_IDT_ENTRY(idt[5],idt_5);    //bound_range_exceeded
    SET_IDT_ENTRY(idt[6],idt_6);    //invalid_opcode
    SET_IDT_ENTRY(idt[7],idt_7);    //device_not_available
    SET_IDT_ENTRY(idt[8],idt_8);    //double_fault
    SET_IDT_ENTRY(idt[9],idt_9);    //coprocessor_segment_overrun
    SET_IDT_ENTRY(idt[10],idt_10);  //invalid_TSS
    SET_IDT_ENTRY(idt[11],idt_11);  //segment_not_present
    SET_IDT_ENTRY(idt[12],idt_12);  //stack_segment_fault
    SET_IDT_ENTRY(idt[13],idt_13);  //general_protection_fault
    SET_IDT_ENTRY(idt[14],idt_14);  //page_fault
    SET_IDT_ENTRY(idt[15],idt_15);  //spurious_interrupt_bug
    SET_IDT_ENTRY(idt[16],idt_16);  //coprocessor_error
    SET_IDT_ENTRY(idt[17],idt_17);  //alignment_check
    SET_IDT_ENTRY(idt[18],idt_18);  //machine_check
    SET_IDT_ENTRY(idt[19],idt_19);  //simd_coprocessor_error
    SET_IDT_ENTRY(idt[keyboard_port],keyboard_a_handler);
//  SET_IDT_ENTRY(idt[40],RTC_WRAPPER);
//  SET_IDT_ENTRY(idt[128],idt_128);
}

/* 
 * idt_0,idt1,idt2......idt_19
 * DESCRIPTION: exceptions handlers
 * INPUTS: none
 * OUTPUTS: prinf error message
 * RETURN VALUE: none
 * SIDE EFFECT: loop in the function
 */

void idt_0(){
    printf("divide_error");
    while(1);
}
void idt_1(){
    printf("debug error");
    while(1);
}
void idt_2(){
    printf("nmi error");
    while(1);
}
void idt_3(){
    printf("breakpoint error");
    while(1);
}
void idt_4(){
    printf("overflow error");
    while(1);
}
void idt_5(){
    printf("bound_range_exceeded error");
    while(1);
}
void idt_6(){
    printf("invalid_opcode error");
    while(1);
}
void idt_7(){
    printf("device_not_available error");
    while(1);
}
void idt_8(){
    printf("double_fault error");
    while(1);
}
void idt_9(){
    printf("coprocessor_segment_overrun error");
    while(1);
}
void idt_10(){
    printf("invalid_TSS error");
    while(1);
}
void idt_11(){
    printf("segment_not_present error");
    while(1);
}
void idt_12(){
    printf("stack_segment_fault error");
    while(1);
}
void idt_13(){
    printf("general_protection_fault error");
    while(1);
}
void idt_14(){
    printf("page_fault error");
    while(1);
}
void idt_15(){
    printf("spurious_interrupt_bug error");
    while(1);
}
void idt_16(){
    printf("coprocessor_error error");
    while(1);
}
void idt_17(){
    printf("alignment_check error");
    while(1);
}
void idt_18(){
    printf("machine_check error");
    while(1);
}
void idt_19(){
    printf("simd_coprocessor_error error");
    while(1);
}



