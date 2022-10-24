#include "rtc.h"
#include "types.h"
#include "lib.h"
#include "i8259.h"

volatile uint8_t rtc_counter;

int32_t interrupt;

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
int rtc_init()
{
    interrupt = 0;
    cli();
    char previ;
    outb(RTCB, RTC_PORT);
    previ = inb(RTC_DATA);
    outb(RTCB, RTC_PORT);    
    outb(previ | 0x40, RTC_DATA);
    enable_irq(IRQ8);
    // rtc_set_rate(MIN_FREQUENCE);
    sti();
    return 0;
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
    outb(0x0C,RTC_PORT); 
    inb(RTC_DATA);
    interrupt=1;
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

int32_t rtc_set_rate(int32_t frequence)
{
    // uint32_t rate;
    // rate = freq2rate(frequence);
    // if (rate == -1) return -1;
    char prev;
    if (isPowerOfTwo(frequence)==0) return -1;
    int32_t fre_index=16-isPowerOfTwo(frequence);
    cli();
    //set index to rtca to stop NMI
    outb(RTCA, RTC_PORT);
    // get value of rtca
    prev = inb(RTC_DATA);
    // reset rtca
    outb(RTCA, RTC_PORT);
    // put our frequence in rtca
    outb((prev & RTC_MASK) | (fre_index & RTC_F), RTC_DATA);
    sti();
    return 0;
}

/*
 *   rtc_open
 *   DESCRIPTION: open the RTC
 *   INPUTS: filename: a pointer to a filename.
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */

int32_t rtc_open(const uint8_t* filename)
{
    rtc_set_rate(MIN_FREQUENCE);
    return 0;
}


/*
 *   rtc_close
 *   DESCRIPTION: close the RTC
 *   INPUTS: fd: a long integer.
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none.
 */
int32_t rtc_close(int32_t fd)
{
    return 0;
}


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
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
	while(interrupt==0);
    interrupt=0;
	return 0;
}


/* int isPowerOfTwo()
*  Description: Checks if it is a power of two and calculate
*  inputs: int num 
*  return value:  i if success , 0 if false
*/

int isPowerOfTwo(int num){
    int i;
    i=0;
    if((num <= 0) || (num > MAX_FREQUENCE) ){
        return 0;
    }
    while(num != 1){
        if(num % 2 != 0) {return i;}
        num /=2;
        i++;
    }
    return i;
}


/*
 *   rtc_write
 *   DESCRIPTION: write RTC rate value.
 *   INPUTS: fd: integer.
 *			 buf: a pointer which stores the frequency with power of two
 *			 nbytes: the bytes that writes per time.
 *   OUTPUTS: none
 *   RETURN VALUE: 0 or -1
 *   SIDE EFFECTS: none.
 */
int32_t rtc_write(int32_t fd, int32_t* buf, int32_t nbytes)
{
    cli();
    if((buf == NULL) ) return -1;
    if (rtc_set_rate(buf[0])==-1){
        printf("please use power of two and smaller than 1024");
    }
    sti();
    return 0;
}



