/* schedule.c  
 */


#include "i8259.h"
#include "lib.h"
#include "schedule.h"



/*
 * pit_init
 *   DESCRIPTION: enable PIT IRQ and set frequency to PIT_FREQ
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void pit_init()
{
  cli();
  int divider = PIT_OSCI_FREQ / PIT_FREQ;

  // Set frequency to PIT_FREQ
  // send low byte
  outb( (divider & 0xFF), PIT_CH0_PORT); 
  // send high byte
  outb( (divider & 0xFF00) >> 8, PIT_CH0_PORT );

  sti();

  enable_irq(PIT_IRQ);
}


void pit_handler()
{
  cli();
  // printf("a");
  int freq = pit_read_freq();
  printf("%d\n",freq);
  sti();
  send_eoi(PIT_IRQ);

}


/*
 * pit_read_freq
 *   DESCRIPTION: read current frequency of PIT
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: latch command is not sent
 *                 RESULT MAY NOT BE ACCURATE!
 */
int pit_read_freq()
{
  int freq;
  cli();
  
  freq = inb(PIT_CH0_PORT);
  freq |= inb(PIT_CH0_PORT) << 8;

  return PIT_OSCI_FREQ / freq;

}
