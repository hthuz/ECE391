
#ifndef _RTC_H
#define _RTC_H

#include "types.h"

#define RTC_PORT  0x70
#define RTC_DATA  0x71

// THREE REGISTERS
#define RTCA  0x8A
#define RTCB  0x8B
#define RTCC  0x8C

#define MAX_FREQUENCE  1024
#define MIN_FREQUENCE  0x02
#define RTC_MASK  0xF0
#define RTC_F 0x0F
#define RTC_RATE  0x02
#define IRQ8  8



extern int rtc_init();

extern void rtc_interrupt();

extern int32_t rtc_set_rate(int32_t frequence);

extern int32_t rtc_open(const uint8_t* filename);

extern int32_t rtc_close(int32_t fd);

extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

extern int32_t rtc_write(int32_t fd, const void * buf, int32_t nbytes);

extern int isPowerOfTwo(int num);
#endif

