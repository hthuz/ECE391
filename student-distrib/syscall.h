/* syscall.h: header file for system call
 */
#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "file.h"
#include "paging.h"

// A header occupies first 40 bytes that gives information about load and starting
#define FHEADER_LEN 40
#define EXE_MAGIC1 0x7f
#define EXE_MAGIC2 0x45
#define EXE_MAGIC3 0x4c
#define EXE_MAGIC4 0x46

#define FARRAY_SIZE 8
#define US_START 0x08000000  // user space start
#define US_END 0x08400000  // user space end

#define K_TASK_STACK_SIZE (P_4K_SIZE * 2)  // task's kernel stack size
#define K_BASE (P_4M_SIZE * 2)  // Base address of kernel
#define PROG_IMAGE_ADDR 0x08048000
#define MAX_PROC_NUM 6

#define CASE_RTC 0
#define CASE_FILE 2
#define CASE_DIR 1
// #define CASE_TERMINAL 3


typedef struct optable_t
{
   int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
  int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
  int32_t (*open)(const uint8_t* filename);
  int32_t (*close)(int32_t fd);
}optable_t;

// File Array Entry structure
typedef struct fentry_t
{
  optable_t* optable_ptr;
  int32_t inode;
  int32_t f_pos;
  int32_t flags;  // 0: inactive 1: active
}fentry_t;

// Process Control Block structure
typedef struct pcb_t
{
  uint32_t pid;
  uint32_t parent_pid;
  fentry_t farray[FARRAY_SIZE];
  uint32_t saved_esp;
  uint32_t saved_ebp;
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

// Get the address of PCB for a process
pcb_t* get_pcb(int32_t pid);

// Initialize all file arrays
void optable_init();

#endif

