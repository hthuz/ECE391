#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

int main()
{
    int fd_391_sound = ece391_open((uint8_t*)"sound");

    if(fd_391_sound == -1) 
    {
        ece391_fdputs(1, (uint8_t*)"could not open this file\n");
        return -1;
    }
    uint32_t playing_sound = 400;
    ece391_write(fd_391_sound, &playing_sound, sizeof(uint32_t));
    playing_sound = 1<<16;
    int i;
    for(i=0;i<100000000; i++){
        ece391_write(fd_391_sound, &playing_sound, sizeof(uint32_t));
    }

    ece391_close(fd_391_sound);
    return 0;
}
