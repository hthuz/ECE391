/* syscall.c: code for 10 system calls
 */
#include "syscall.h"
#include "file.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"

#define SYSCALL_FAIL -1;

// Initially, there is no process, denote as -1
int32_t cur_pid = ROOT_PID;

int task_num = 0;
// An array keeps track of all active tasks
int running_tasks[MAX_TASK_NUM] = {0};

optable_t stdin_optable;
optable_t stdout_optable;
optable_t rtc_optable;
optable_t file_optable;
optable_t dir_optable;

/*
 * halt
 *   DESCRIPTION: halt a program, if it is the process 0 shell,
                  reset cur_pid and execute the shell again
 *   INPUTS: status -- name of the result to return eax
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t halt(uint8_t status)
{
  cli();

  int i;
  pcb_t *cur_pcb = get_pcb(cur_pid);
  pcb_t* parent_pcb = get_pcb(cur_pcb->parent_pid);
  termin_t* running_term = get_terminal(running_tid);


  // Restore parent data
  // if it is the original shell
  if(cur_pcb->parent_pid == -1)
  {
    printf("Can't Exit Base Shell\n");
    cur_pid = ROOT_PID;
    running_tasks[cur_pcb->pid] = 0;
    task_num--;
    execute((uint8_t *)"shell");
  }

  // Note now cur_pid has become parent_pid
  // But cur_pcb doesn't change
  free_pid(cur_pid);

  cur_pid = cur_pcb->parent_pid;


  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE - cur_pid * K_TASK_STACK_SIZE - sizeof(int32_t);
  // Restore parent paging
  set_process_paging(cur_pid); // flushing TLB has been contained.
  // Close any relevant FDs
  for (i = 0; i < FARRAY_SIZE; i++)
  {
    (cur_pcb->farray[i]).flags = 0; // 0 means inactive
  }

  // close the relevant video memory
  if (cur_pcb->use_vid == 1)
  {
    cur_pcb->use_vid = 0;
    reset_vidmap_paging();
  }

  // Update terminal information
  running_term->pid = cur_pid;

  // Jump to execute return
  uint32_t my_esp = parent_pcb->saved_esp;
  uint32_t my_ebp = parent_pcb->saved_ebp;
  uint32_t result = (uint32_t)status;
  sti();
  asm volatile(
      "movl %%ebx, %%ebp    ;"
      "movl %%ecx, %%esp    ;"
      "movl %%edx, %%eax    ;"
      "leave;"
      "ret;"
      :
      : "c"(my_esp), "b"(my_ebp), "d"(result)
      : "eax", "ebp", "esp");
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
int32_t execute(const uint8_t *command)
{
  cli();
  pcb_t *pcb;               // PCB of program
  int32_t parent_pid;       // Record of parent id
  int32_t new_pid;          // new pid for this newly executed program
  uint8_t usr_cmd[ARG_LEN];
  uint8_t usr_args[ARG_LEN];
  termin_t* cur_term = get_terminal(cur_tid);


  if(parse_args(command, usr_cmd, usr_args) == -1)
  {
    sti();
    return -1;
  }

  if(check_exec(usr_cmd) == -1)
  {
    sti();
    return -1;
  }

  // record parentid and current id
  if(-1 == ( new_pid = create_pid() ))
  {
    sti();
    return -1;
  }

   
  if(term_switch_flag == 1)
  {
    parent_pid = ROOT_PID;
    term_switch_flag = 0;

    pcb_t* cur_pcb = get_pcb(cur_pid);
    register uint32_t saved_ebp asm("ebp");
    register uint32_t saved_esp asm("esp");
    cur_pcb->saved_ebp = saved_ebp;
    cur_pcb->saved_esp = saved_esp;
  }
  else
  {
    parent_pid = cur_pid;
  }

  // Change to new process
  cur_pid = new_pid;

  // Update the process running in current terminal
  cur_term->pid = cur_pid;
  // Current terminal should be set to running immediately
  running_tid = cur_tid;

  set_process_paging(cur_pid);

  pcb = create_pcb(cur_pid,parent_pid, usr_args);

  context_switch(usr_cmd);

  return 0;
}

/*
 *  int32_t read(int32_t fd, void* buf, int32_t nbytes)
 *  DESCRIPTION: call the read function based on the file type
 *  INPUTS: int32_t fd  - the file descriptor
 *          void* buf - ptr to the buffer
 *          int32_t nbytes - the length of the bytes to read
 *  OUTPUTS: read function
 *  RETURN VALUE: return fd for SUCCESS, -1 for FAIL
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
  pcb_t *curr = get_pcb(cur_pid);
  // if fd=0 or fd is out of the range return SYSCALL_FAIL
  if (fd == 1 || fd < 0 || fd > FARRAY_SIZE)
    return SYSCALL_FAIL;
  // check if ptr is in user space
  if ((int)buf < US_START || (int)buf + nbytes > US_END)
    return SYSCALL_FAIL;
  // check flag and read function
  if (!curr->farray[fd].flags)
    return SYSCALL_FAIL;
  if (curr->farray[fd].optable_ptr->read == NULL)
    return SYSCALL_FAIL;

  return curr->farray[fd].optable_ptr->read(fd, buf, nbytes); //(fd,file_array[fd].file_position,buf,nbytes)
}

/*
 *  write(int32_t fd, const void* buf, int32_t nbytes)
 *  DESCRIPTION: call the write function
 *  INPUTS: int32_t fd  - the file descriptor
 *          void* buf - ptr to the buffer
 *          int32_t nbytes - the length of the bytes to write
 *  OUTPUTS: write function
 *  RETURN VALUE: return fd for SUCCESS, -1 for FAIL
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
  pcb_t *curr = get_pcb(cur_pid);
  // some check to avoid invalid situations
  if (fd <= 0 || fd >= FARRAY_SIZE || buf == NULL)
    return SYSCALL_FAIL;
  if (!curr->farray[fd].flags)
    return SYSCALL_FAIL;
  if (curr->farray[fd].optable_ptr->write == NULL)
    return SYSCALL_FAIL;
  // check if ptr is in user space
  if ((int)buf < US_START || (int)(buf + nbytes) > US_END)
    return SYSCALL_FAIL;
  return curr->farray[fd].optable_ptr->write(fd, buf, nbytes);
}

/*
 *  int32_t open (const uint8_t* filename)
 *  DESCRIPTION: fill the one free entry of file array with the file
 *  INPUTS: the file name which want to open
 *  OUTPUTS: NONE
 *  RETURN VALUE: return fd for success, -1 for FAIL
 */
int32_t open(const uint8_t *filename)
{
  int i;
  pcb_t *curr = get_pcb(cur_pid);
  dentry_t curr_dentry;
  // init fd to be -1
  int fd = -1;
  // check if filename is valid to open
  if (filename == NULL || (int)filename < US_START || (int)filename > US_END)
    return SYSCALL_FAIL;
  // check if the filename is exist by read_dentry_by_name function
  if (read_dentry_by_name(filename, &curr_dentry) == -1)
  {
    printf("we cannot find such file");
    return SYSCALL_FAIL;
  }
  // find the table which is not using
  for (i = 0; i < FARRAY_SIZE; i++)
  {
    if (curr->farray[i].flags == 0)
    {
      fd = i;
      break;
    }
  }
  // if fd is -1, no descripto is free now
  if (fd == -1)
  {
    printf("no descriptor is free now");
    return SYSCALL_FAIL;
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
  }
  curr->farray[fd].optable_ptr->open(filename);
  return fd;
}

/*
 *  int32_t close (int32_t fd)
 *  DESCRIPTION: the system call function to close
 *  INPUTS: the file name which it want to closse
 *  OUTPUTS: file array filled with entry
 *  RETURN VALUE: should return fd for SUCCESS, -1 for SYSCALL_FAIL
 */
int32_t close(int32_t fd)
{
  pcb_t *curr = get_pcb(cur_pid);
  // first check; return SYSCALL_FAIL for invailid index, try to closed default or orig closed
  if (fd <= 1 || fd >= FARRAY_SIZE || curr->farray[fd].flags == 0)
    return SYSCALL_FAIL;

  // closed
  curr->farray[fd].flags = 0; // 0 means inactive
  return curr->farray[fd].optable_ptr->close(fd);
}

/*
 * getargs
 *   DESCRIPTION: get arguments from command
 *   INPUTS: buf -- buf to contain arguments
 *           nbytes -- length of buf
 *   OUTPUTS: none
 *   SIDE EFFECTS: none
 */

int32_t getargs(uint8_t *buf, int32_t nbytes)
{
  // check if in user space
  if ((int)buf < US_START || (int)buf + nbytes >= US_END || buf == NULL)
    return SYSCALL_FAIL;

  // check if there are no arguments
  pcb_t *curr = get_pcb(cur_pid);
  if (curr->args[0] == '\0')
    return SYSCALL_FAIL;
  strncpy((int8_t *)buf, (int8_t *)curr->args, nbytes);
  return 0;
}

/*
 *  int32_t vidmap (uint8_t** screen_start)
 *  DESCRIPTION: check the false cases, set the vidmap_paging and set the screen_start
 *  INPUTS: the pointer screen_start, it stores the virtual address of start of video memory
 *  OUTPUTS: 0
 *  RETURN VALUE: should return 0 for SUCCESS, -1 for SYSCALL_FAIL
 */
int32_t vidmap(uint8_t **screen_start)
{
  if (screen_start == NULL)
    return SYSCALL_FAIL;
  if ((uint32_t)screen_start < US_START || (uint32_t)screen_start > US_END)
    return SYSCALL_FAIL;
  // 128MB is the start of the program image
  // set 140MB as the virtual space address of memory
  set_vidmap_paging();
  pcb_t *pcb = get_pcb(cur_pid);
  pcb->use_vid = 1;
  (*screen_start) = (uint8_t *)(35 * P_4M_SIZE);
  return 0;
}

/* To be done */
int32_t set_handler(int32_t signum, void *handler_address)
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

  flush_tlb();

}

/*
 * set_vidmap_paging
 *   DESCRIPTIOIN: set vidmap paging for program
 *   INPUTS: pid -- pid of process to set up paging
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_vidmap_paging()
{
  int index = PDE_INDEX(35 * P_4M_SIZE);
  // 140MB is the virtual space address of memory
  p_dir[index].present = 1;
  p_dir[index].page_size = 0;
  p_dir[index].u_su = 1;
  p_dir[index].base_addr = (((int)video_p_table) >> 12);

  video_p_table[0].base_addr = VID_MEM_START >> 12; // page is 4k aligned,
                                                    // the address is multiple of 4k
                                                    // so lower 12 bits not required
  video_p_table[0].present = 1;

  flush_tlb();
}

void hide_term_vid_paging(int32_t tid)
{
  int index = PDE_INDEX(35 * P_4M_SIZE);
  
  // 140MB is the virtual space address of memory
  p_dir[index].present = 1;
  p_dir[index].page_size = 0;
  p_dir[index].u_su = 1;
  p_dir[index].base_addr = (((int)video_p_table) >> 12);

  video_p_table[0].base_addr = TERM_VID_ADDR(tid) >> 12; // page is 4k aligned,
                                                    // the address is multiple of 4k
                                                    // so lower 12 bits not required
  video_p_table[0].present = 1;

  flush_tlb();
}
/*
 * reset_vidmap_paging
 *   DESCRIPTIOIN: reset vidmap paging for program
 *   INPUTS: pid -- pid of process to set up paging
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void reset_vidmap_paging()
{
  int index = PDE_INDEX(35 * P_4M_SIZE);
  // 140MB is the virtual space address of memory
  p_dir[index].present = 0;
  p_dir[index].page_size = 0;
  p_dir[index].u_su = 0;
  p_dir[index].base_addr = 0;

  video_p_table[0].base_addr = 0; // page is 4k aligned,
                                  // the address is multiple of 4k
                                  // so lower 12 bits not required
  video_p_table[0].present = 0;
  // do not need to flush TLB?????
}

/*
 * get_pcb
 *   DESCRIPTION:get the one process's PCB
 *   INPUTS: pid -- pid of process to get PCB
 *   OUTPTUS: none
 *   RETURN VALUE: address of PCB of process
 *   SIDE EFFECTS: none
 */
pcb_t *get_pcb(int32_t pid)
{
  return (pcb_t *)(P_4M_SIZE * 2 - (pid + 1) * P_4K_SIZE * 2);
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

/*
 * parse_args
 *   DESCRIPTION: Helper function for execute
 *                Given command, divide into to command part and argumetns part
 *   INPUTS: command -- user type
 *           usr_cmd -- an array to contain user command
 *           usr_args -- an array to contain user arguments
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if command is invalid
 *                  0 otherwise
 *   SIDE EFFECTS: results are stored in usr_cmd and usr_args
 */
int parse_args(const uint8_t *command, uint8_t* usr_cmd, uint8_t* usr_args)
{
  int i;
  uint32_t cmd_len = 0;
  uint32_t args_len = 0;
  uint32_t command_length = strlen((int8_t*) command);

  if (command == NULL)
    return -1;

  // Empty usr_cmd and usr_args first  
  for(i = 0; i < ARG_LEN; i++)
  {
    usr_cmd[i] = '\0';
    usr_args[i] = '\0';
  }
  i = 0;

  while (command[i] == ' ')
    i++;
  while (command[i] != ' ' && command[i] != '\0' && i <= command_length)
    usr_cmd[cmd_len++] = command[i++];
  usr_cmd[cmd_len] = '\0';

  if (command[i] == ' ')
  {
    while (command[i] == ' ')
      i++;
    while (command[i] != ' ' && command[i] != '\0' && i <= command_length)
      usr_args[args_len++] = command[i++];
    usr_args[args_len] = '\0';
  }
  return 0;
}


/*
 * check_exec
 *   DESCRIPTION: Helper function for execute
 *                Check if user command is valid, executable etc
 *   INPUTS: usr_cmd -- user command
 *   OUTPUTS: none
 *   RETURN VALUE: -1 if user command is invalid/not executable
 *                  0 if valid
 *   SIDE EFFECTS: none
 */ 
int check_exec(uint8_t* usr_cmd)
{
  dentry_t dentry;          // dentry of program file
  nodes_block *inode;       // inode of program file
  uint8_t buf[FHEADER_LEN]; // buf containing bytes of the file

  // Check user comnand
  if (read_dentry_by_name(usr_cmd, &dentry) == -1)
    return -1;
  inode = (nodes_block *)(mynode + dentry.inode);
  if (dentry.filetype != FILE_TYPE)
    return -1;
  if (read_data(dentry.inode, 0, buf, FHEADER_LEN) != FHEADER_LEN)
    return -1;
  if (buf[0] != EXE_MAGIC1 || buf[1] != EXE_MAGIC2 ||
      buf[2] != EXE_MAGIC3 || buf[3] != EXE_MAGIC4)
    return -1;

  return 0;

}


/*
 * create_pcb
 *   DESCRIPTION: Helper function for execute
 *                Given process id and user_args, set up PCB
 *                for this Process
 *   INPUTS: pid -- process id for corresponding PCB
 *           parent_pid -- parent_pid for this PCB
 *           usr_args -- user arguments for this PCB
 *   OUTPUTS: none
 *   RETURN VALUE: the created pcb
 *   SIDE EFFECTS: none
 */
pcb_t* create_pcb(int32_t pid, int32_t parent_pid,  uint8_t* usr_args)
{
  int i;
  pcb_t* pcb;

  pcb = get_pcb(pid);
  pcb->pid = pid;
  pcb->parent_pid = parent_pid;

  for(i = 0; i < ARG_LEN; i++)
    pcb->args[i] = '\0';
  if (strlen((const int8_t*)usr_args) > 0)
  {
    for (i = 0; i <= strlen((const int8_t*)usr_args); i++)
      pcb->args[i] = usr_args[i];
  }
  pcb->use_vid = 0;

  // Initialize File array
  for (i = 0; i < FARRAY_SIZE; i++)
  {
    pcb->farray[i].flags = 0;
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
  if(parent_pid == -1)
  {
    pcb->saved_ebp = saved_ebp;
    pcb->saved_esp = saved_esp;
  }else
  {
    pcb_t* parent_pcb = get_pcb(pcb->parent_pid);
    parent_pcb->saved_ebp = saved_ebp;
    parent_pcb->saved_esp = saved_esp;
  }

  return pcb;
}


/*
 * context_switch
 *   DESCRIPTION: Helper function for execute
 *                Load the program
 *                Do Context Switch staff
 *   INPUTS: usr_cmd -- name of the program to execute
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: should be used after check_exec 
 *                 as this function doesn't not check anything
 */
void context_switch(uint8_t* usr_cmd)
{
  dentry_t dentry;
  nodes_block *inode;       // inode of program file
  uint8_t buf[FHEADER_LEN]; // buf containing bytes of the file
  int32_t entry_pt;         // Entry point into the program (EIP)
                            
  read_dentry_by_name(usr_cmd, &dentry);
  inode = (nodes_block *)(mynode + dentry.inode);
  read_data(dentry.inode, 0, buf, FHEADER_LEN);
  
  // Load file into program image
  read_data(dentry.inode, 0, (uint8_t *)PROG_IMAGE_ADDR, inode->length);

  // Entry point is stored in bytes 24-27 of the executable
  entry_pt = (buf[27] << 24) | (buf[26] << 16) | (buf[25] << 8) | (buf[24]);

  int32_t usr_esp;
  usr_esp = P_128M_SIZE + P_4M_SIZE - sizeof(int32_t);

  tss.ss0 = KERNEL_DS;
  tss.esp0 = K_BASE - (cur_pid) * K_TASK_STACK_SIZE - sizeof(int32_t);

  sti();
  // push iret context to kernel stack
  // reference: https://wiki.osdev.org/getting_to_ring_3
  asm volatile(
      "movw %%ax, %%ds;"
      "pushl %%eax;"
      "pushl %%ebx;"
      "pushfl;"
      "pushl %%ecx;"
      "pushl %%edx;"
      "iret;"
      :
      : "a"(USER_DS), "b"(usr_esp), "c"(USER_CS), "d"(entry_pt)
      : "memory");
}



/*
 * create_pid
 *   DESCRIPTION: Create one pid for new task
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: created pid from free pids if success
 *                 -1 if fail
 *   SIDE EFFECTS: Will mark one additional process as running
 *
 */
int32_t create_pid()
{
  int pid = 0;

  if(task_num == MAX_TASK_NUM)
  {
    printf("Can't Create More Processes\n");
    return -1;
  }
  

  for(pid = 0; pid < MAX_TASK_NUM; pid++)
  {
    if(running_tasks[pid] == 0)
    {
      running_tasks[pid] = 1;
      break;
    }
  }

  task_num++;
  return pid;
}

/*
 * free_pid
 *   DESCRIPTOIN: Free one pid from running tasks
 *                after one task halts
 *   INPUTS: pid -- pid to be freed
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void free_pid(int32_t pid)
{
  running_tasks[pid] = 0;
  task_num--;
}
