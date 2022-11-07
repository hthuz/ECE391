/* syscall.c: code for 10 system calls
 */
#include "syscall.h"
#include "file.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"

#define succsess 0;
#define fail -1;

// Initially, there is no process, denote as -1
int32_t cur_pid = -1;

extern pde_t p_dir[PDE_NUM] __attribute__((aligned (P_4K_SIZE)));
extern nodes_block* mynode;

optable_t stdin_optable;
optable_t stdout_optable;   //operation table for stdin and stdout
optable_t rtc_optable;
optable_t file_optable;
optable_t dir_optable;

int32_t halt(uint8_t status)
{ 
    int i;
    pcb_t* cur_pcb = get_pcb(cur_pid);
    // Restore parent data
    // if it is the original shell
    if ( cur_pid == 0 ){
      printf("Can't Exit Base Shell\n");
      cur_pid = -1;
      execute((uint8_t*)"shell");
    }

    // Note now cur_pid has become parent_pid
    cur_pid = cur_pcb->parent_pid;
    
    tss.ss0= KERNEL_DS;
    tss.esp0= K_BASE - cur_pid * K_TASK_STACK_SIZE - sizeof(int32_t);
    // Restore parent paging
    set_process_paging(cur_pid);   //flushing TLB has been contained.
    // Close any relevant FDs
    for (i=0;i < FARRAY_SIZE;i++){
      (cur_pcb->farray[i]).flags=0;
    }
    // Jump to execute return
    uint32_t my_esp=cur_pcb->saved_esp;
    uint32_t my_ebp=cur_pcb->saved_ebp;
    uint32_t result= (uint32_t) status;
    asm volatile ( 
            "movl %%ebx, %%ebp    ;"
            "movl %%ecx, %%esp    ;"
            "movl %%edx, %%eax    ;"
            "leave;"
            "ret;"
            :
            :"c"(my_esp),"b"(my_ebp),"d"(result)
            :"eax","ebp","esp"
    );
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
  pcb_t* pcb;                 // PCB of program
  int32_t parent_pid;         // Record of parent id
  dentry_t dentry;            // dentry of program file
  nodes_block* inode;         // inode of program file
  uint8_t buf[FHEADER_LEN];   // buf containing bytes of the file
  int32_t entry_pt;           // Entry point into the program (EIP)
  int32_t user_esp;           // ESP for user program
  int32_t i;                  // Index for loop

  // 1. Parse args
  if(command == NULL)
    return -1;
  if(read_dentry_by_name(command, &dentry) == -1)
    return -1;
  inode = (nodes_block*) (mynode + dentry.inode);

  // 2. Check for executable
  if(dentry.filetype != FILE_TYPE)
    return -1;
  if(read_data(dentry.inode, 0, buf, FHEADER_LEN) != FHEADER_LEN)
    return -1;
  if(buf[0] != EXE_MAGIC1 || buf[1] != EXE_MAGIC2 ||
     buf[2] != EXE_MAGIC3 || buf[3] != EXE_MAGIC4)
    return -1;
  
  // record parentid and current id
  parent_pid = cur_pid;
  cur_pid++;

  if(cur_pid == MAX_PROC_NUM)
  {
    cur_pid--;
    printf("Can't Create More Processes\n");
    return -1;
  }

  // 3. Set up paging
  set_process_paging(cur_pid);

  // 4. Load file into program image 
  read_data(dentry.inode,0,(uint8_t*)PROG_IMAGE_ADDR,inode->length);

  // 5. Create PCB
  pcb = get_pcb(cur_pid);
  pcb->pid = cur_pid;
  pcb->parent_pid = parent_pid;

  // Initialize File array
  // flag 0: inactive
  // flag 1: active
  for(i = 0; i < FARRAY_SIZE; i++)
  {
    pcb->farray[i].flags = 0 ;
  }
  // File array for stdin
  pcb->farray[0].optable_ptr = &stdin_optable;
  pcb->farray[0].flags = 1;

  // File array for stdout
  pcb->farray[1].optable_ptr = &stdout_optable;
  pcb->farray[1].flags = 1;

  // Store EBP and ESP
  register uint32_t saved_ebp asm("ebp");
  register uint32_t saved_esp asm("esp");
  pcb->saved_ebp = saved_ebp;
  pcb->saved_esp = saved_esp;

  // 6. Prepare for Context Switch  
  user_esp = P_128M_SIZE + P_4M_SIZE - sizeof(int32_t);
  // Entry point is stored in bytes 24-27 of the executable
  // Use these four bytes as EIP
  entry_pt = (buf[27] << 24) | (buf[26] << 16) | (buf[25] << 8) | (buf[24]);

  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE - (pcb->pid) * K_TASK_STACK_SIZE - sizeof(int32_t);
  
  // 7. Push IRET context to kernel stack
  // Reference: https://wiki.osdev.org/Getting_to_Ring_3
  asm volatile(
      "movw %%ax, %%ds;"
      "pushl %%eax;"
      "pushl %%ebx;"
      "pushfl;"
      "pushl %%ecx;"
      "pushl %%edx;"
      "iret;"
      :
      :"a"(USER_DS),"b"(user_esp),"c"(USER_CS),"d"(entry_pt)
      :"memory"
  );
  return 0;
}

/*
 *  int32_t read(int32_t fd, void* buf, int32_t nbytes)
 *  DESCRIPTION: call the read function based on the file type             
 *  INPUTS: int32_t fd  - the file descriptor
 *          void* buf - ptr to the buffer
 *          int32_t nbytes - the length of the bytes to read
 *  OUTPUTS: read function
 *  RETURN VALUE: return fd for SUCCESS, -1 for fail
 */ 
int32_t read(int32_t fd, void* buf, int32_t nbytes)
{
  pcb_t* curr = get_pcb(cur_pid);
  // if fd=0 or fd is out of the range return fail
  if(fd==1 || fd<0 || fd>FARRAY_SIZE) return fail;
  //check if ptr is in user space
  if((int)buf<US_START || (int)buf+nbytes>US_END) return fail;
  //check flag and read function
  if(!curr->farray[fd].flags) return fail;
  if(curr->farray[fd].optable_ptr->read==NULL) return fail;
  
  return curr->farray[fd].optable_ptr->read(fd,buf,nbytes); //(fd,file_array[fd].file_position,buf,nbytes)
}

/*
 *  write(int32_t fd, const void* buf, int32_t nbytes)
 *  DESCRIPTION: call the write function          
 *  INPUTS: int32_t fd  - the file descriptor
 *          void* buf - ptr to the buffer
 *          int32_t nbytes - the length of the bytes to write
 *  OUTPUTS: write function
 *  RETURN VALUE: return fd for SUCCESS, -1 for fail
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes)
{
  pcb_t* curr = get_pcb(cur_pid);
  // some check to avoid invalid situations
  if(fd<=0 || fd>=FARRAY_SIZE || buf==NULL) return fail;
  if(!curr->farray[fd].flags) return fail;
  if(curr->farray[fd].optable_ptr->write==NULL) return fail;
  //check if ptr is in user space
  if((int)buf<US_START || (int)(buf+nbytes)>US_END) return fail;
  return curr->farray[fd].optable_ptr->write(fd,buf,nbytes);
}

/*
 *  int32_t open (const uint8_t* filename)
 *  DESCRIPTION: fill the one free entry of file array with the file                
 *  INPUTS: the file name which want to open
 *  OUTPUTS: NONE
 *  RETURN VALUE: return fd for success, -1 for fail
 */
int32_t open(const uint8_t* filename)
{
  int i;
  pcb_t* curr = get_pcb(cur_pid);
  dentry_t curr_dentry;
  // init fd to be -1
  int fd = -1; 
  // check if filename is valid to open
  if(filename == NULL || (int)filename<US_START || (int)filename>US_END) return fail;
  // check if the filename is exist by read_dentry_by_name function
  if(read_dentry_by_name(filename, &curr_dentry)==-1) 
  {
    printf("we cannot find such file");
    return fail;
  }
  // find the table which is not using
  for(i=0; i<FARRAY_SIZE; i++)
  {
    if(curr->farray[i].flags == 0)
    {
      fd =i;
      break;
    }
  }
  // if fd is -1, no descripto is free now 
  if(fd==-1) 
  {
    printf("no descriptor is free now");
    return fail;
  }
  // according to the found fd, set position and flag
  curr->farray[fd].f_pos = 0;
  curr->farray[fd].flags = 1;


  switch (curr_dentry.filetype)
  {
  case CASE_RTC:
    curr->farray[fd].inode = -1;
    curr->farray[fd].optable_ptr = &rtc_optable;
    break;
  case CASE_FILE:
    curr->farray[fd].inode = curr_dentry.inode;
   curr->farray[fd].optable_ptr = &file_optable;
    break;
  case CASE_DIR:
    curr->farray[fd].inode = curr_dentry.inode;
   curr->farray[fd].optable_ptr = &dir_optable;
    break;
  // case CASE_TERMINAL:
  //   curr->farray[fd].inode = curr_dentry.inode;
  //   curr->farray[fd].optable_ptr->open = &terminal_open;
  //   curr->farray[fd].optable_ptr->close = &terminal_close;
  //   curr->farray[fd].optable_ptr->read = &terminal_read;
  //   curr->farray[fd].optable_ptr->write = &terminal_write;
  //   break;
  }
  curr->farray[fd].optable_ptr->open(filename);
  return fd;
}

/*
 *  int32_t close (int32_t fd)
 *  DESCRIPTION: the system call function to close                
 *  INPUTS: the file name which it want to closse
 *  OUTPUTS: file array filled with entry
 *  RETURN VALUE: should return fd for SUCCESS, -1 for fail
 */
int32_t close(int32_t fd)
{
  pcb_t* curr = get_pcb(cur_pid);
  // first check; return fail for invailid index, try to closed default or orig closed
  if( fd<=1 || fd>=FARRAY_SIZE || curr->farray[fd].flags==0) return fail;
  
  // closed
  curr->farray[fd].flags = 0; // 0 means inactive
  return curr->farray[fd].optable_ptr->close(fd);
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

  // Set 4M paging for process 
  // Process 0: 8MB - 12MB
  // Process 1: 12MB - 16MB
  // and so on
  index = PDE_INDEX(P_128M_SIZE);
  p_dir[index].present = 1;
  p_dir[index].page_size = 1;
  p_dir[index].cache_dis = 1;
  p_dir[index].u_su = 1;
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

/*
 * get_pcb
 *   DESCRIPTION:get the one process's PCB
 *   INPUTS: pid -- pid of process to get PCB
 *   OUTPTUS: none
 *   RETURN VALUE: address of PCB of process
 *   SIDE EFFECTS: none
 */
pcb_t* get_pcb(int32_t pid)
{
  return (pcb_t*)(P_4M_SIZE * 2 - (pid + 1) * P_4K_SIZE * 2); 
}

/*
 * optable_init
 *   DESCRIPTION: initialize all operation tables
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void optable_init()
{
  stdin_optable.open = terminal_open;
  stdin_optable.read = terminal_read;
  stdin_optable.write = 0;
  stdin_optable.close = terminal_close;

  stdout_optable.open = terminal_open;
  stdout_optable.read = 0;
  stdout_optable.write = terminal_write;
  stdout_optable.close = terminal_close;

  rtc_optable.open = rtc_open;
  rtc_optable.close = rtc_close;
  rtc_optable.read = rtc_read;
  rtc_optable.write = rtc_write;

  file_optable.open = file_open;
  file_optable.close = file_close;
  file_optable.read = file_read;
  file_optable.write = file_write;

  dir_optable.open = directory_open;
  dir_optable.close = directory_close;
  dir_optable.read = directory_read;
  dir_optable.write = directory_write;
  
}
