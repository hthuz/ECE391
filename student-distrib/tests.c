#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"

#define PASS 1
#define FAIL 0
#define KEYBOARD_IDT_FAIL 2

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
	while(1);
}

/* IDT Test
 * 
 * Asserts that first 20 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL/KEYBOARD_FAIL/RTC_FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: idt.c
 */
int idt_test_totally(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i){
		//printf("i=%d:%#x    \n",i,idt[i].offset_15_00);
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	//printf("end loop\n");
	// test if the keyboard handlers works
	if ((idt[33].offset_15_00 == NULL) && 
		(idt[33].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	// test if the rtc handlers works
	/*
	if ((idt[40].offset_15_00 == NULL) && 
		(idt[40].offset_31_16 == NULL)){
			assertion_failure();
			result = RTC_IDT_FAIL;
		}
	*/
	return result;
	while(1);
}


/* i8259 Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: i8259 double input
 * Files: i8259.c
 */
int i8259_test(){
	TEST_HEADER;
	int result = PASS;
	printf("initial master_mask=%u   \n",get_master_mask());
	printf("inital slave_mask=%u    \n",get_slave_mask());
	enable_irq(8);
	printf(" enable-8 master_mask=%u    \n",get_master_mask());
	printf(" enable-8 slave_mask=%u     \n",get_slave_mask());
	disable_irq(1);
	printf(" disable-1 master_mask=%u    \n",get_master_mask());
	printf(" disable-1 slave_mask=%u     \n",get_slave_mask());
	return result;
	while(1);
}

/* Checkpoint 2 tests */
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_test_totally", idt_test_totally());
	TEST_OUTPUT("i8259_test", i8259_test());
	// launch your tests here
}
