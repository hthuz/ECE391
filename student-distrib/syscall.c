/* syscall.c: code for 10 system calls
 */
#include "syscall.h"
#include "file.h"
#include "lib.h"





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
  dentry_t dentry; 
  int32_t i;
  
  // Parse args
  if(command == NULL)
    return -1;

  if(read_dentry_by_name(command, &dentry) == -1)
    return -1;

  // Check for executable
  if(dentry.filetype != FILE_TYPE)
    return -1;

  // Check if txt file
  uint8_t* cmd;
  for(i = 0; i < strlen(command); i++)
  {
    cmd[i] = command[i];
  }
  cmd = strrev(cmd); 
  if(strncmp(cmd, "txt", 3) == 0)
    return -1;

  // Set up paging
  

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


