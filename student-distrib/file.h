#ifndef _FILE_H
#define _FILE_H

#include "types.h"
// magic number of file.h
#define length_of_data_index        1023
#define boot_reserved     13
#define length_of_dir_entries       63
#define dentry_reserved 6

// magic number of file.c
#define number_of_mynode   100
#define Four_KB            4096
#define NameLen            32
#define default_fd         2
#define FILE_TYPE          2
#define DIR_TYPE           1

// type def
typedef struct dentry_t        //64B
{
    uint8_t filename[32];      // uint8_t=char
    uint32_t filetype;              
    uint32_t inode;                  //index nodes
    uint32_t reserved[dentry_reserved];            //24B reserved
}dentry_t;        // the actual unit is 4bytes

typedef struct boot_block       //4KB
{
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint32_t reserved[boot_reserved];
    dentry_t dir_entries[length_of_dir_entries];  //one block has 4KB=64*64B
}boot_block;

typedef struct nodes_block
{
    uint32_t length;                                 // the total bytes of it
    uint32_t data_index[length_of_data_index];       // size of pointer is 4B
}nodes_block;   

typedef struct data_block
{
    uint8_t data[4096];    // 4B*1024=1B*4096
}data_block;   

typedef struct open_file_table  
// this struct must be an array of 8 for each process? not sure
// use file decriptor, the integer index to point to the specific table
{
    uint32_t* opera_tab_pointer;
    uint32_t  inode;
    uint32_t  file_position;
    uint32_t  flags;              // 1 means in-use, 0 means not use it 
}open_file_table;   

// file init
extern void fs_init_address(uint32_t address);
void init_file_table(int32_t fd);

// file functions
int file_open(const uint8_t* fname);
int file_close(int32_t fd);
int file_write();
int file_read(int32_t fd, uint32_t count, uint8_t* buf);

//directory functions
int directory_open(const uint8_t* fname);
int directory_close();
int directory_write();
int directory_read(const uint8_t* fname);

// helper functions

int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);


// useless function
// extern int test_file();
// extern boot_block* get_myboot();
// extern nodes_block* get_mynode();

#endif /* _FILE_H */
