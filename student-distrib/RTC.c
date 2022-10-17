#include "RTC.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"

volatile uint8_t rtc_counter;

enum interrupt_t{
	inted,
	uninted
}inter;

// struct of rtc table
/*
typedef struct rtc_table {
	int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
	int32_t (*write)(int32_t fd, const void* buf, int32_t nbyte);
	int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
}rtc_table_t;
rtc_table_t rtcjumptable;
*/

/*
 *   rtc_init
 *   DESCRIPTION: initialize the RTC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: update values of register A and B in RTC
 */
void rtc_init()
{
    //inter = uninted;
    /*
    rtcjumptable.read = rtc_read;
	rtcjumptable.write = rtc_write;
	rtcjumptable.open = rtc_open;
	rtcjumptable.close = rtc_close;
    */
    cli();
    char previ;
    outb(RTCB, RTC_PORT);
    previ = inb(RTC_DATA);
    outb(RTCB, RTC_PORT);    
    outb(previ | 0x40, RTC_DATA);
    enable_irq(IRQ8);
    //rtc_set_rate(MIN_FREQUENCE);
    sti();
}


/*
 *   rtc_interrupt
 *   DESCRIPTION: used when periodic RTC interrupt recieved
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change register C to enable next interrupt signal
 */
void rtc_interrupt()
{
    cli();
    //printf("come here");
    // increse the counter
    rtc_counter ++;
    //test_interrupts();
    // throwaway the value of rtcc.
    outb(0x0C,RTC_PORT); 
    inb(RTC_DATA);

    //inter = inted;

    sti();
    // send end
    send_eoi(IRQ8); 

}


/*
 *   rtc_set_rate
 *   DESCRIPTION: set RTC rate according to input requence.
 *   INPUTS: input: the frequence we want
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */
/*
int32_t rtc_set_rate(int32_t frequence)
{
    char prev;
    cli();
    //set index to rtca to stop NMI
    outb(RTCA, RTC_PORT);
    // get value of rtca
    prev = inb(RTC_DATA);
    // reset rtca
    outb(RTCA, RTC_PORT);
    // put our frequence in rtca
    outb((prev & RTC_MASK) | (frequence & RTC_F), RTC_PORT);
    sti();
}
*/


/*
 *   rtc_open
 *   DESCRIPTION: open the RTC
 *   INPUTS: filename: a pointer to a filename.
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */
/*
int32_t rtc_open(const uint8_t* filename)
{
    return 0;
}
*/

/*
 *   rtc_close
 *   DESCRIPTION: close the RTC
 *   INPUTS: fd: a long integer.
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */

/*
int32_t rtc_close(int32_t fd)
{
    rtc_set_rate(MIN_FREQUENCE);
    return 0;
}
*/

/*
 *   rtc_read
 *   DESCRIPTION: read RTC.
 *   INPUTS: fd: integer.
 *			 buf: a pointer which stores the frequency
 *			 nbytes: the bytes that writes per time.
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */
/*
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
    sti();
	// disable other all interrupts 
	disable_irq(0);
	disable_irq(1);
    // wait for interruption
	inter = uninted;
	while(inter == inted);
	// enable other all interrupts
	enable_irq(0);
	enable_irq(1);
	return 0;
*/

/* int isPowerOfTwo()
*  Description: Checks if it is a power of two and calculate
*  inputs: int num 
*  return value:  i if success , 0 if false
*/
/*
int isPowerOfTwo(int num){
    int i;
    i=0;
    if((num <= 0) || (num > MAX_FREQUENCE) || ((int)num != num)){
    return 0;
    }
    while(num != 1){
    if(num % 2 != 0) {return 0;}
    num /=2;
    i++;
    }
    return i;
}
*/

/*
 *   rtc_write
 *   DESCRIPTION: write RTC rate value.
 *   INPUTS: fd: integer.
 *			 buf: a pointer which stores the frequency
 *			 nbytes: the bytes that writes per time.
 *   OUTPUTS: none
 *   RETURN VALUE: nbytes or -1
 *   SIDE EFFECTS: none.
 */

/*
int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes)
{
    cli();
    int freq;
    int w;
    if((buf == NULL) || (nbytes != 4)) return -1;
    freq = *(int*)buf;
    w = isPowerOfTwo(freq);
    if(w == 0) return -1;
    else{
        rtc_set_rate(16-w);
    }
    sti();
    return nbytes;
}
*/
