#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "paging.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

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

/* paging_init_test
 * DESCRIPTIONS: test paging initialize
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: PASS if no problem
 *          	  FAIL otherwise  
 * SIDE EFFECTS: none
 */
int paging_init_test()
{
	int i;
	char* addr;
	char value;

	// test video memory
	addr = (char*) VID_MEM_START;
	value = *addr;

	addr = (char*) (VID_MEM_START + P_4K_SIZE  - 1);
	printf("memory video end addr is %x\n", addr);
	value = *addr;

	// test kernel memory
	addr = (char*) KERNEL_ADDR;
	value = *addr;

	addr = (char*) KERNEL_ADDR;
	printf("kernel memory end address is %x\n",addr);
	value = *addr;

	// test present
	if(p_dir[0].present == 0)
	{
		printf("directory entry 0 not present!\n");
		return FAIL;
	}
	if(p_table[PTE_INDEX(VID_MEM_START)].present == 0)
	{
		printf("video memory table not present!\n");
		return FAIL;
	}


	// print to see values of directory and table
	printf("Page Directroy as follows: (index, base_addr\n");
	for(i = 0; i < PDE_NUM; i++)
  	{
    	printf("%d  , %d  \n",i, p_dir[i].base_addr);
  	}	
  	printf("Page table as follows:(index,base_addr)\n");
  	for(i = 0; i < PTE_NUM; i++)
  	{
    	printf("%d  , %d  \n ", i, p_table[i].base_addr);
  	}
	return PASS;
}


/* i8259 Test
 * 
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: i8259 double input
 * Files: i8259.c
 */
/*
int mult_test(){
	int* addr;
	int value;
	addr = (int*)(VID_MEM_START + P_4K_SIZE - 1);
	value - *addr;
	int a=4;
	int b=0;
	int c=a/b;
	return a;
}
*/
/* Checkpoint 2 tests */


/* Termial Test
 *
 *   Asserts that terminal read and write works
 *   INPUTS: none
 *   OUTPUTS: PASS/FAIL
 *   SIDE EFFECTS: None
 *   Coverage: Terminal read, terminal write
 *   Files: terminal.h/cs
 */


int terminal_test()
{
	TEST_HEADER;
	int result = PASS;

	char writestr[] = "This is STRING for testing terminal write\n";
	int32_t fd = 3;
	uint8_t* filename = 0;
	char buf[KB_BUF_SIZE + 1] = {'\0'};
	
	terminal_open(filename);
	if ( terminal_write(fd,writestr, strlen(writestr)) != strlen(writestr))
	{
		assertion_failure();
		result = FAIL;
	}
	
	// test termianl read
	while(1)
	{
		// no need to read '/n'
		int num;
		// printf("[391OS@localhost]$ ");
		num = terminal_read(fd, buf, kb_buf_length - 1);
		if(num == KB_BUF_SIZE)
		{
			putc('\n');
		}
		terminal_write(fd, buf, strlen(buf));
		putc('\n');
	}



	return result;
	
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_test_totally", idt_test_totally());
	//TEST_OUTPUT("divide test", divide_test());
	//TEST_OUTPUT("paging_init_test", paging_init_test());
	TEST_OUTPUT("terminal test", terminal_test());
	// launch your tests here
}
