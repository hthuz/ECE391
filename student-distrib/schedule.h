/* schedule.h
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H


#define PIT_IRQ 0x00
#define PIT_CH0_PORT 0x40
#define PIT_CMD_PORT 0x43
#define PIT_OSCI_FREQ 1193182
// #define PIT_FREQ 100
#define PIT_MAX_FREQ 1193180
#define PIT_FREQ (PIT_MAX_FREQ / 20)


void pit_init();
void pit_handler();
int pit_read_freq();

void task_switch();
void set_running_terminal();




#endif



