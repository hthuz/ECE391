#ifndef _FILE_H
#define _FILE_H

#include "types.h"
// magic number of file.h
#define length_of_data_index        4092
#define boot_reserved     13
#define length_of_dir_entries       63
#define dentry_reserved 6

// magic number of file.c
#define number_of_mynode   100
#define Four_KB            4096
// type def
typedef struct dentry_t        //64B
{
    uint8_t filename[32];      // uint8_t=char
    int filetype;              
    int inode;                  //index nodes
    int reserved[dentry_reserved];            //24B reserved
}dentry_t;        // the actual unit is 4bytes

typedef struct boot_block       //4KB
{
    int num_dir_entries;
    int num_inodes;
    int num_data_blocks;
    int reserved[boot_reserved];
    dentry_t dir_entries[length_of_dir_entries];  //one block has 4KB=64*64B
}boot_block;

typedef struct nodes_block
{
    int length;
    int data_index[length_of_data_index];       // size of pointer is 4B
}nodes_block;   

typedef struct data_block
{
    uint8_t data[4096];    // 4B*1024=1B*4096
}data_block;   

// typedef struct open_file_table  
// // this struct must be an array of 8 for each process? not sure
// // use file decriptor, the integer index to point to the specific table
// {
//     int* opera_tab_pointer;
//     int  inode;
//     int  file_position;
//     int  flags;              // 1 means in-use, 0 means not use it 
// }open_file_table;   

// file init
extern void fs_init_address(uint32_t address);
// file test
extern int test_file();
// file functions
int file_open();
int file_close();
int file_write();
int file_read();

//directory functions
int directory_open();
int directory_close();
int directory_write();
int directory_read();

// helper functions
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

#endif /* _FILE_H */
