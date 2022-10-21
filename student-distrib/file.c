#include "file.h"
#include "types.h"
#include "lib.h"

boot_block* myboot;
nodes_block* mynode;
// open_file_table my_file_table[8];
data_block* mydata;
dentry_t* tempdentry;  // the temporary dentry

void fs_init_address(uint32_t address){
    myboot= (boot_block*) address;
    mynode= (nodes_block*) (address+Four_KB);
    mydata= (data_block*) (address+Four_KB+Four_KB* (myboot->num_inodes));
    // printf("boot_block adress is:%x\n",myboot);
}

// helper functions

/*
 * DESCRIPTION: 
 *          "return -1" indicating a non-existent file, 
 *          if the name is valid, fill the dentry with file 
 *          name, file type and inode number from the 
 *          boot_block that has the same name, then return 0.
 * 
 * INPUTS:  fname: the name of the file, find it from the boot_block
 *          dentry: the pointer of the dentry struct
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, -1 on failure
 * SIDE EFFECT: none   
*/
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    int i,j;
    uint32_t mylength=strlen( (int8_t*) fname);
    // check the -1 case
    if ( fname==NULL) return -1;
    if ( mylength >32 ) return -1;
    // search the name in the boot_block
    for (i=0;i<myboot->num_dir_entries;i++){
        // hislength=strlen(myboot.dir_entries[i].filename);
        // if (mylength!=hislength ) continue;
                // compare the two names
        printf("%d %d",i,myboot->num_dir_entries);
        if (strncmp( (int8_t*)myboot->dir_entries[i].filename,  (int8_t*)fname, mylength)==0){
            dentry->filetype=myboot->dir_entries[i].filetype;
            dentry->inode=myboot->dir_entries[i].inode;
            for (j=0;j < mylength;j++){
                dentry->filename[j]=fname[j];
            }
            return 0;
        }
    }
    return -1;      // if not found
}

/*
 * DESCRIPTION: 
 *          "return -1" indicating a non-existent index, 
 *          if the name is valid, fill the dentry with file 
 *          name, file type and inode number from the 
 *          boot_block that has the same name, then return 0.
 * 
 * INPUTS:  index: the index of the file, find it from the boot_block
 *          dentry: the pointer of the dentry struct
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, -1 on failure
 * SIDE EFFECT: none   
*/
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    int i,j;
    int hislength;
    // check the -1 case
    if (index >= myboot->num_dir_entries )  return -1;
    // copy
        // hislength=strlen(myboot.dir_entries[i].filename);
        // if (mylength!=hislength ) continue;
                // compare the two names
    dentry->filetype=myboot->dir_entries[i].filetype;
    dentry->inode=myboot->dir_entries[i].inode;
    
    hislength=strlen ( (int8_t*)myboot->dir_entries[index].filename );
    for (j=0;j < hislength;j++){
        dentry->filename[j]=myboot->dir_entries[index].filename[j];
    }

    return 0;
}

/*
 * DESCRIPTION: 
 *          reading up to length bytes starting from position 
 *          offset in the file with inode number inode and 
 *          returning the number of bytes read and placed in 
 *          the buffer. data block is not continuous!!!!!
 * 
 * INPUTS:  inode: the index of the node among the nodes
 *          offset: the offset of the file
 *          buf: destination buffer
 *          length: the read bytes length
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, -1 on failure
 * SIDE EFFECT: read data, write into buffer   
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    nodes_block* the_node=(nodes_block*) (mynode+inode);
    data_block* the_data;
    uint32_t mylength=the_node->length;
    int idata;
    int sindex;
    int roffset;
    int j;
    int count=0;
    int index_count=0;
    // if it is out of upperbound
    if( Four_KB* mylength -1 < offset + length) return -1;
    // |0~4095|4096~8191|8192~12287|12288~16383|
    // |   0  |    1    |    2     |     3     |
    // However, it's not continuous
    sindex=offset/Four_KB;
    roffset = offset-sindex*Four_KB;

    if ( roffset+length <= Four_KB){
        idata=(the_node->data_index[sindex]);
        the_data= (data_block*) (mydata+idata);

        for (j=roffset ; j< roffset+length ; j++){
            buf[count]=the_data->data[j];
            count++;
        }
    }

    else{
        idata=(the_node->data_index[sindex]);
        the_data= (data_block*) (mydata+idata);

        for (j=roffset ; j< Four_KB ; j++){
            buf[count]=the_data->data[j];
            count++;
            length--;
        }

        while ( length>=Four_KB){
            index_count++;
            idata=(the_node->data_index[sindex+index_count]);
            the_data= (data_block*) (mydata+idata);

            for (j=0 ; j< Four_KB ; j++){
                buf[count]=the_data->data[j];
                count++;
                length--;
            }
        }

        index_count++;
        idata=(the_node->data_index[sindex+index_count]);
        the_data= (data_block*) (mydata+idata);

        for (j=0 ; j< length ; j++){
            buf[count]=the_data->data[j];
            count++;
            length--;
        }
    }
    return 0;
}


int file_open(const uint8_t* fname){
    if (read_dentry_by_name (fname, tempdentry)==-1) return -1;
    return 0;
}

int file_close(int32_t fd){
    return 0;
}

int file_write(){
    return -1;
}

int file_read(uint32_t count, uint8_t* buf){
    if (tempdentry->filetype!=2) return -1;
    read_data (tempdentry->inode, 0, buf, count);
    return 0;
}

int directory_open(const uint8_t* fname){
    // printf("come to directory open  ");
    if (read_dentry_by_name (fname, tempdentry)==-1) return -1;
    return 0;
}

int directory_close(int32_t fd){
    return 0;
}

int directory_write(){
    return -1;
}

int directory_read(const uint8_t* fname){
    // printf("\ncome to dir_read  ");
    if (tempdentry->filetype!=2) return -1;
    printf("directory name is:%s",tempdentry->filename);
    return 0;
}

int test_file(){
    // test the address
        // int i;
        // printf("myboot information:  \n");
        // printf("   num_dir_entries: %d  \n",myboot->num_dir_entries);
        // printf("   num_inodes: %d  \n",myboot->num_inodes);
        // printf("   num_data_blocks: %d  \n",myboot->num_data_blocks);
        // for (i=0;i<myboot->num_dir_entries;i++){
        //     printf("   dentry_name[%d]:%s            \n",i,myboot->dir_entries[i].filename);
        // }
    //test the read_dentry_by_name
    


    return 0;
}

