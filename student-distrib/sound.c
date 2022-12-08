/*
	Reference: wiki.osdev.org PC speaker section
*/
#include "sound.h"
#include "schedule.h"

void play_sound(uint32_t nFrequence){
	uint32_t Div;
 	uint8_t tmp;
 
 	Div = 1193180 / nFrequence;
 	outb(0xb6,PIT_CMD_PORT);
 	outb((uint8_t) (Div & 0xFF), PIT_CH2_PORT);
 	outb((uint8_t) (Div >> 8), PIT_CH2_PORT);
 
 	tmp = inb(0x61);
  	
	if (tmp != (tmp | 3))
 	outb( (tmp | 3), 0x61);
}
 
void nosound(){
	cli();
	uint8_t tmp = inb(0x61) & 0xFC;
 	outb(tmp, 0x61);
	sti();
}

// void beep() {
//     int32_t i = 0;
//     play_sound(1000);
//     while(i<1000){
//         i++;
//     }
//     nosound();
//  }

// default
int32_t sound_open(const uint8_t* filename)
{
    return 0;
}

// default
int32_t sound_close(int32_t fd)
{
    return 0;
}

// default
int32_t sound_read(int32_t fd, void* buf, int32_t nbytes)
{
    return 0;
}

int32_t sound_write(int32_t fd, const void* buf, int32_t nbytes)
{
    if((buf ==NULL)) return FAIL;
    if(nbytes != sizeof(uint32_t)) return FAIL;

    uint32_t value = *((uint32_t*)buf);
    uint32_t freq = value & 0xFF;
    uint32_t play_or_not = (value>>8) & 0xFF;

    if (play_or_not == 0)
    {
        play_sound(freq);
    }else if(play_or_not == 1){
        nosound();
    }else{
        return FAIL;
    }
    return 0;
}

