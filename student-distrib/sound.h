#ifndef _SOUND_H
#define _SOUND_H

#include "i8259.h"
#include "lib.h"
#include "types.h"
#include "rtc.h"
#include "paging.h"

#define FAIL  -1

extern void play_sound(uint32_t nFrequence);
extern void nosound();
// extern void beep();
extern void beep_init();
extern int32_t sound_open(const uint8_t* filename);
extern int32_t sound_close(int32_t fd);
extern int32_t sound_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t sound_write(int32_t fd, const void* buf, int32_t nbytes);
 
#endif
