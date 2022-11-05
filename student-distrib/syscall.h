/* syscall.h: header file for system call
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "file.h"

#define EXE_MAGIC_NUM 4
#define EXE_MAGIC1 0x7f
#define EXE_MAGIC2 0x45
#define EXE_MAGIC3 0x4c
#define EXE_MAGIC4 0x46


#define PROG_IMAGE 0x08048000

// File Array Entry structure
typedef struct fentry_t
{
  int32_t optable_ptr;
  int32_t inode;
  int32_t f_pos;
  int32_t flags;
}fentry_t;

// Process Control Block structure
typedef struct pcb_t
{
  uint32_t pid;
  uint32_t parent_pid;
  fentry_t farray[8];
  

}pcb_t;

// System call functions
int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);


// Helper functions
// Set up paging for a process
void set_process_paging(int32_t pid);



#endif

