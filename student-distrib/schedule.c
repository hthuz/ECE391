/* schedule.c  
 */


#include "i8259.h"
#include "x86_desc.h"
#include "lib.h"
#include "schedule.h"
#include "syscall.h"
#include "terminal.h"

// Current Termianl that scheduling is running
// Note difference with cur_tid;
int32_t running_tid = 0;
// Number of terminals executed in one round
int32_t term_num_oneround = 1;
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
  termin_t* running_term;
  termin_t* next_term;
  int32_t next_pid;
  pcb_t* cur_pcb;
  pcb_t* next_pcb;

  running_term = get_terminal(running_tid);
  cur_pid =  running_term->pid_list[running_term->pid_num - 1];
  cur_pcb = get_pcb(cur_pid);

  set_running_terminal();

  next_term = get_terminal(running_tid);
  next_pid = running_term->pid_list[running_term->pid_num - 1];
  next_pcb = get_pcb(next_pid);


  // Store cur_pid's EBP,ESP
  register uint32_t saved_ebp asm("ebp");
  register uint32_t saved_esp asm("esp");
  cur_pcb->saved_ebp = saved_ebp;
  cur_pcb->saved_esp = saved_esp;


  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE - next_pid * K_TASK_STACK_SIZE - sizeof(int32_t);
  set_process_paging(next_pid);
  // Use next task's EBP,ESP
  uint32_t ebp = next_pcb->saved_ebp;
  uint32_t esp = next_pcb->saved_esp;
  asm volatile(
      "movl %%ebx, %%ebp;"
      "movl %%ecx, %%esp;"
      "leave;"
      "ret;"
      :
      :"b"(ebp),"c"(esp)
      :"ebp","esp"
      );

  return;
}

void set_running_terminal()
{
  int i;
  if(term_num_oneround == term_num)
  {
    term_num_oneround = 1;
    running_tid = 0;
    return;
  }

  for(i = 0; i < MAX_TASK_NUM; i++)
  {
    if (terminals[i].invoked == 1 && i > running_tid)
    {
      running_tid = i;
      term_num_oneround++;
      break;
    }
  }
  return;

}
