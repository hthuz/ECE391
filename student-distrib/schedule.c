/* schedule.c  
 */


#include "i8259.h"
#include "x86_desc.h"
#include "lib.h"
#include "schedule.h"
#include "syscall.h"
#include "terminal.h"

// Number of terminals executed in one round
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
  int divider = PIT_OSCI_FREQ / PIT_FREQ;

  // send low byte
  outb( (divider & 0xFF), PIT_CH0_PORT); 
  // send high byte
  outb( (divider & 0xFF00) >> 8, PIT_CH0_PORT );

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


/*
 * pit_handler
 *   DESCRIPTION: hander function for PIT, calls task_switch
 *                every time an interrupt occurs
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void pit_handler()
{
  send_eoi(PIT_IRQ);
  task_switch();
  return;
}

/*
 * task_switch
 *   DESCRIPTION: shift to next active termianl and switch
 *                to next task.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE_EFFECTS: will change the running task
 */
void task_switch()
{

  // if only one terminal, no need to switch
  if(term_num == 1)
    return;

  cli();
  termin_t* curr_running_term;
  termin_t* next_term;
  int32_t next_pid;
  pcb_t* cur_pcb;
  pcb_t* next_pcb;


  curr_running_term = get_terminal(running_tid);
  cur_pcb = get_pcb(curr_running_term->pid);

  // After this function, running_tid is changed
  set_running_terminal();

  next_term = get_terminal(running_tid);
  next_pid = next_term->pid;

  next_pcb = get_pcb(next_pid);

   // If running termianl is current terminal, show it
  if (running_tid == cur_tid)
     set_vidmap_paging();
  else
     hide_term_vid_paging(running_tid);

  set_process_paging(next_pid);

  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE  - next_pid * K_TASK_STACK_SIZE - sizeof(int32_t);

  cur_pid = next_pid;


  // Store cur_pid's EBP,ESP
  register uint32_t saved_ebp asm("ebp");
  register uint32_t saved_esp asm("esp"); 
  cur_pcb->saved_ebp = saved_ebp;
  cur_pcb->saved_esp = saved_esp;


  // Context switch
  // Use next task's EBP,ESP
  uint32_t next_ebp = next_pcb->saved_ebp;
  uint32_t next_esp = next_pcb->saved_esp;
    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        :"r"(next_esp), "r"(next_ebp)
    );
  sti();


}


/*
 * set_running_terminal
 *   DESCRIPTION: change current running terminal next one
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: will update running_tid to next terminal's tid
 */
void set_running_terminal()
{
  int i;

  for(i = 0 ; i < MAX_TERM_NUM; i++)
  {
    running_tid++;
    running_tid = running_tid % MAX_TERM_NUM;
    if(terminals[running_tid].invoked == 1)
      break;
  }

}
