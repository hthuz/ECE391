/* syscall.c: code for 10 system calls
 */
#include "syscall.h"
#include "file.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"


int32_t cur_pid = 0;
extern pde_t p_dir[PDE_NUM] __attribute__((aligned (P_4K_SIZE)));
extern nodes_block* mynode;



int32_t halt(uint8_t status)
{
  return 0;
}

/*
 * execute
 *   DESCRIPTION: load and execute a program, handing off
 *                the processor to new program until it terminates
 *   INPUTS: command -- name of executeable program to execute
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if command can't be executed
 *                 256 if programs dies by exception
 *                 0-255 if program executes a halt system call
 *   SIDE EFFECTS: none
 */
int32_t execute(const uint8_t* command)
{
  pcb_t* pcb;        // PCB of program
  optable_t stdin_optable;
  optable_t stdout_optable;  //operation table for stdin and stdout
  dentry_t dentry;  // dentry of program file
  nodes_block* inode; // inode of program file
  uint8_t buf[FHEADER_LEN];     // buf containing bytes of the file
  int32_t entry_pt;     // Entry point into the program (EIP)
  
  // Parse args
  if(command == NULL)
    return -1;
  if(read_dentry_by_name(command, &dentry) == -1)
    return -1;
  inode = (nodes_block*) (mynode + dentry.inode);

  // Check for executable
  if(dentry.filetype != FILE_TYPE)
    return -1;
  if(read_data(dentry.inode, 0, buf, FHEADER_LEN) != FHEADER_LEN)
    return -1;
  if(buf[0] != EXE_MAGIC1 || buf[1] != EXE_MAGIC2 ||
     buf[2] != EXE_MAGIC3 || buf[3] != EXE_MAGIC4)
    return -1;

  // Set up paging
  set_process_paging(cur_pid);

  // Load file into program image 
  read_data(dentry.inode,0,(uint8_t*)PROG_IMAGE_ADDR,inode->length);

  // Create PCB
  pcb = (pcb_t*)(P_4M_SIZE * 2 - (cur_pid + 1) * P_4K_SIZE * 2); 
  
  if(cur_pid == 0)
    pcb->parent_pid = 0;
  else
    pcb->parent_pid = cur_pid - 1;
  pcb->pid = cur_pid;
  cur_pid++;

  // File array for stdin
  stdin_optable.open = &terminal_open;
  stdin_optable.read = &terminal_read;
  stdin_optable.write = 0;
  stdin_optable.close = &terminal_close;
  pcb->farray[0].optable_ptr = &stdin_optable;
  pcb->farray[0].flags = 1;

  // File array for stdout
  stdout_optable.open = &terminal_open;
  stdout_optable.read = 0;
  stdout_optable.write = &terminal_write;
  stdout_optable.close = &terminal_close;
  pcb->farray[1].optable_ptr = &stdout_optable;
  pcb->farray[1].flags = 1;
  
  // Prepare for Context Switch  
  // Entry point is sotred in bytes 24-27 of the executable
  entry_pt = (buf[24] << 24 ) | (buf[25] << 16) | (buf[26] << 8) | buf[27]


  



  return 0;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
  
  return 0;
}

int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{

  return 0;
}

int32_t open(const uint8_t* filename)
{

  return 0;
}

int32_t close(int32_t fd)
{

  return 0;
}

/* To be done */
int32_t getargs(uint8_t* buf, int32_t nbytes)
{

  return 0;
}

/* To be done */
int32_t vidmap(uint8_t** screen_start)
{

  return 0;
}

/* To be done */
int32_t set_handler(int32_t signum, void* handler_address)
{

  return 0;
}

/* To be done */
int32_t sigreturn(void)
{

  return 0;
}

/*
 * set_process_paging
 *   DESCRIPTIOIN: set up 4MB paging for program
 *   INPUTS: pid -- pid of process to set up paging
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_process_paging(int32_t pid)
{
  int index;
  index = PDE_INDEX(P_128M_SIZE);
  p_dir[index].present = 1;
  p_dir[index].page_size = 1;
  p_dir[index].cache_dis = 1;
  p_dir[index].base_addr = (((pid + 2) * P_4M_SIZE) >> 12);

  // flush TLB
  asm volatile (
    "movl %%cr3, %%eax;"
    "movl %%eax, %%cr3;"
    :
    :
    :"%eax"
  );

}



