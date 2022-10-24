#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "i8259.h"
#include "paging.h"
#include "terminal.h"
#include "file.h"

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


/* file_content
 * DESCRIPTIONS: print the file content
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: PASS if no problem
 *          	  FAIL otherwise  
 * SIDE EFFECTS: you can set the minimum unit
 */
int file_content(){
	TEST_HEADER;
	int32_t fd=2;					// default fd
	int32_t result=0;
	int32_t unit=1;
	const char* st="frame1.txt";
	// const char* st2="frame1.txt";
	uint8_t buffer[unit+1];
	buffer[unit]='\0';			// you should initialize with \0
								// to help printf
	clear();
	
	// the first file
	if(file_open((const uint8_t* )st)==-1) {
		printf("wrong name");
		return FAIL;
	}
	while (1){
		result=file_read(fd,unit,buffer);
		// putc(buffer[0]);
		printf("%s",buffer);
		if(result==1) break;
	}
	printf("file_name: %s\n",st);
	file_close(fd);			// close fd, close my_file_table[2], init the value in it

	// // the second file
	// if(file_open((const uint8_t* )st2)==-1) {
	// 	printf("wrong name");
	// 	return FAIL;
	// }
	// while (1){
	// 	result=file_read(fd,unit,buffer);
	// 	if(result==1) break;
	// 	printf("%s",buffer);
	// 	// putc(buffer[0]);
	// }
	// printf("file_name: %s\n",st2);
	// file_close(fd);

	return PASS;
	while(1);
}

/* nullbytes_file_content
 * DESCRIPTIONS: use putc to print the file with null bytes
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: PASS if no problem
 *          	  FAIL otherwise  
 * SIDE EFFECTS: you can set the minimum unit, you can use i to see ELF
 */
int nullbytes_file_content(){
	TEST_HEADER;
	// int32_t i=0;			//used to see ELF
	int32_t fd=2;					// default fd
	int32_t result=0;
	int32_t unit=1;
	const char* st="ls";
	// const char* st2="frame1.txt";
	uint8_t buffer[unit+1];
	buffer[unit]='\0';			// you should initialize with \0
								// to help printf
	clear();
	
	// the first file
	if(file_open((const uint8_t* )st)==-1) {
		printf("wrong name");
		return FAIL;
	}
	while (1){
		result=file_read(fd,unit,buffer);
		// putc(buffer[0]);
		putc(buffer[0]);
		// i++;
		// if (i==10) break;
		if(result==1) break;
	}
	printf("file_name: %s\n",st);
	//printf("\n");
	file_close(fd);			// close fd, close my_file_table[2], init the value in it
	return PASS;
	while(1);
}

/* file_list
 * DESCRIPTIONS: print the file list using directory_open
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUES: PASS if no problem
 *          	  FAIL otherwise  
 * SIDE EFFECTS: none
 */
int file_list(){
	TEST_HEADER;
	const char* st=".";
	if(directory_open((const uint8_t* )st)==-1) {
		printf("wrong dir name");
		return FAIL;
	}
	directory_read((const uint8_t*)st);
	return PASS;
	while(1);
}

/* RTC_open_test
 * Discription: test whether RTC_open works fine.
 * Inputs: None
 * Outputs: '1' are put on the screen periodically.
 * Side Effects: '1' is put on the screen in a specific frequency if RTC_open works.
 */
int rtc_open_test(){
	uint8_t* temp_open =0;
	if(rtc_open(temp_open) == 0){
		return PASS;
	}
	return 0;
	// while(1){
	// 	rtc_read(0,0,0);
	// 	putc('3');
	// }
}

// int rtc_open_test2(){
// 	uint8_t* temp_open =0;
// 	rtc_open(temp_open);
// 	return PASS;
// }

/* RTC_write_test
 * Discription: test whether RTC_write works fine.
 * Inputs: freq: the freq that we want to set to the RTC.
 * Outputs: '1' are put on the screen periodically.
 * Side Effects: none.
 */
void rtc_write_test(int freq){
		rtc_write(0,(int*)&freq,4);
		while(1){
			rtc_read(0,0,0);
			putc('1');
		}
}


// void test_rtc_open_close(){
// 	int check_close = 1;
// 	int count = 0;
// 	uint32_t rtc_buf;
// 	TEST_HEADER;
// 	rtc_open(0);
// 	/*Print ten '1' to the screen*/
// 	while (count < 10){
// 		rtc_read(0, &rtc_buf,4);
// 		putc('1');
// 		count++;
// 	}
// 	/*check whether the RTC_close will return 0*/
// 	check_close = rtc_close(0);
// 	if (!check_close){
// 		printf("\nRTC_close returns 0 successfully");
// 	}
// }


int rtc_test() {
    unsigned char freq[1];
	int char_count;
	int i;
	uint8_t * temp_open  = (uint8_t *) 0;
	TEST_HEADER;

    rtc_open(temp_open);

	int char_bound = 7; //Initially print out six chars of "1" 
	// Loop to try out all frequencies
	for(i = 0; i <13; i++){
		freq[0] = 15 - i;
		char_count = 0; 
		// loop to print '1' based on the frequency
		while(char_count < char_bound){
			putc('1');
			// temporary use freq as the buf and sizeof for rtc_read since no full implementation yet
			rtc_read(0,freq, 4);
			char_count++;
			// char_bound = char_bound + 1; 
		}
		// print new line after finish printing for 1 frequency
		putc('\n');
		//As the freq of rtc increases, the screen update more faster, thus putc more "1" to recognize it
		char_bound = char_bound +1; // A random formula to increase the bound for char to print.
		rtc_write(0,freq, 4);
	}
	rtc_close(0);
	return PASS;
}




/* RTC_close_test
 * Discription: test whether RTC_close works fine.
 * Inputs: none.
 * Outputs: print "rtc close test success" if passes test.
 * Side Effects: none.
 */
void rtc_close_test(){
	if(!rtc_close(0))
		printf("rtc close test success\n");
}

int test_selffunction(int num){
	printf("the answer is %d", isPowerOfTwo(num));
	return 0;
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
	// TEST_OUTPUT("terminal test", terminal_test());

	// TEST_OUTPUT("file_content", file_content());
	// TEST_OUTPUT("nullbytes_file_content", nullbytes_file_content());
	TEST_OUTPUT("file list", file_list());

	/*used for checkpoint2 of rtc*/
	// TEST_OUTPUT("rtc_open", rtc_open_test());
	// rtc_close_test();
	// TEST_OUTPUT("rtc_write", rtc_write_test());
	// rtc_write_test(256);
	// test_selffunction(24);
	TEST_OUTPUT("rtc_test", rtc_test());
	// test_rtc_open_close();
	// rtc_test();
	// launch your tests here
}
