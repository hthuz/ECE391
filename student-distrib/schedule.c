/* schedule.c  
 */


#include "i8259.h"
#include "x86_desc.h"
#include "lib.h"
#include "schedule.h"
#include "syscall.h"


int32_t running_pid = -1;
// Number of tasks executed in one round
int32_t task_num_oneround = 0;
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


void pit_handler()
{
  send_eoi(PIT_IRQ);
  task_switch();
  return;
}

void task_switch()
{
  set_running_pid();
  pcb_t* running_pcb = get_pcb(running_pid);

  register uint32_t saved_esp asm ("esp");
  register uint32_t saved_ebp asm ("ebp");
  
  running_pcb->saved_ebp = saved_ebp;
  running_pcb->saved_esp = saved_esp;

  // Context Switch
  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE - (running_pid) * K_TASK_STACK_SIZE - sizeof(int32_t);
  set_process_paging(running_pid);

  uint32_t esp = running_pcb->saved_esp;
  uint32_t ebp = running_pcb->saved_ebp;

  asm volatile(
      "movl %%ebx, %%ebp    ;"
      "movl %%ecx, %%esp    ;"
      "leave;"
      "ret;"
      :
      : "c"(esp), "b"(ebp)
      : "ebp","esp");

  return;
}

void set_running_pid()
{
  int i;
  if(task_num_oneround == task_num)
  {
    task_num_oneround = 0;
    running_pid = -1;
  }

  for(i = 0; i < MAX_TASK_NUM; i++)
  {
    if (running_tasks[i] == 1 && i > running_pid)
    {
      running_pid = i;
      task_num_oneround++;
      break;
    }
  }
  return;

}
