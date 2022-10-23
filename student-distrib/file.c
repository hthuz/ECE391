#include "file.h"
#include "types.h"
#include "lib.h"

boot_block* myboot;
nodes_block* mynode;
open_file_table my_file_table[8];
data_block* mydata;


/*
 * DESCRIPTION: 
 *          get the file system address from kernel.c, the module 0.
 * INPUTS:  uint32_t address
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: set the address of myboot,mynode and mydata,
 *              the pointer of the three areas. Call init_file_table.
*/
void fs_init_address(uint32_t address){
    myboot= (boot_block*) address;
    mynode= (nodes_block*) (address+Four_KB);
    mydata= (data_block*) (address+Four_KB+Four_KB* (myboot->num_inodes));
    init_file_table(default_fd);
}

/*
 * DESCRIPTION: 
 *          reset my_file_table[fd]
 * INPUTS:  fd: the file descriptor
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: initialize the my_file_table[fd]
*/
void init_file_table(int32_t fd){
    my_file_table[fd].file_position=0;
    my_file_table[fd].flags=0;
    my_file_table[fd].inode=0;
    return;
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
    int32_t i;
    uint32_t mylength=strlen( (int8_t*) fname);
    // uint32_t hislength;
    // check the -1 case
    if ( fname==NULL) return -1;
    if ( mylength > NameLen ){
        // printf("this file name is invalid  \n");
        // return -1;
        mylength=NameLen;
    }
    // search the name in the boot_block
    for (i=0;i<myboot->num_dir_entries;i++){
                // compare the two names
        if (strncmp( (const int8_t*)myboot->dir_entries[i].filename, (const int8_t*)fname, mylength)==0){
            dentry->filetype=(myboot->dir_entries[i]).filetype;
            dentry->inode=(myboot->dir_entries[i]).inode;
            strncpy((int8_t*)dentry->filename,(int8_t*)fname,NameLen);
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

    // check the -1 case
    if (index >= myboot->num_dir_entries ) {
        printf("index out of range");
        return -1;
    }

    dentry->filetype = (myboot->dir_entries[index]).filetype;
    dentry->inode = (myboot->dir_entries[index]).inode;
    strncpy((int8_t*)dentry->filename,(int8_t*)myboot->dir_entries[index].filename,NameLen);
    

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
 * RETURN VALUE: count: the number of bytes I read, 
 *               0: come to the file end
 *               -1: invalid offset
 * SIDE EFFECT: read data, write into buffer   
*/
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    nodes_block* the_node=(nodes_block*) (mynode+inode);
    data_block* the_data;
    uint32_t totallength=the_node->length;         
    uint32_t idata;
    uint32_t sindex;
    uint32_t roffset;
    uint32_t j;
    uint32_t count=0;
    uint32_t index_count=0;
    uint32_t mylen=length;
    uint32_t temp;
    // |0~4095|4096~8191|8192~12287|12288~16383|
    // |   0  |    1    |    2     |     3     |
    // However, it's not continuous, 
    // it's just used to illustrate the question
    sindex=offset/Four_KB;
    roffset = offset-sindex*Four_KB;

    // if offset invalid
    if( totallength  <= offset ) {
        printf("offset invalid");
        return -1;
    }
        // if not cross over the block
    if ( roffset+length <= Four_KB){
        idata=(the_node->data_index[sindex]);
        // printf("idata is:%d\n",the_node->data_index[sindex]);
        the_data= (data_block*) (mydata+idata);
        for (j=roffset ; j< roffset+length ; j++){
            buf[count]=the_data->data[j];
            //putc(buf[count]);
            count++;
            if (totallength==offset+count) {
                buf[count]='\0';    // printf stop earlier using \0
                return 0;           //come to the end
            }
        }     
    }

        // if cross over the block
    else{
        // copy the first block
        idata=(the_node->data_index[sindex]);
        the_data= (data_block*) (mydata+idata);

        for (j=roffset ; j< Four_KB ; j++){
            buf[count]=the_data->data[j];
            count++;
            mylen--;
            if (totallength==offset+count) {
                buf[count]='\0';    
                return 0;          
            }
        }

        // copy the middle whole block into the buffer
        while ( mylen>=Four_KB){
            index_count++;
            idata=(the_node->data_index[sindex+index_count]);
            the_data= (data_block*) (mydata+idata);

            for (j=0 ; j< Four_KB ; j++){
                buf[count]=the_data->data[j];
                count++;
                mylen--;
                if (totallength==offset+count) {
                    buf[count]='\0';    
                    return 0;          
                }
            }
        }
        //   printf("mylen=%d\n",mylen);
        //   printf("count=%d\n",count);

        // copy the left elements
        index_count++;
        idata=(the_node->data_index[sindex+index_count]);
        the_data= (data_block*) (mydata+idata);

        temp=mylen;     // store the length now 
        for (j=0 ; j< temp ; j++){
            buf[count]=the_data->data[j];
            count++;
            mylen--;
            if (totallength==offset+count) {
                buf[count]='\0';    
                return 0;          
            }
        }
           //printf("final length=%d\n",mylen);
           //printf("final count=%d\n",count);
    }
    return count;

}


/*
 * DESCRIPTION: 
 *          then check if filename valid
 *          if valid, set my_file_table[2].
 * INPUTS:  fname
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, 1 if fail:wrong name/wrong type
 * SIDE EFFECT: none
*/
int file_open(const uint8_t* fname){
    int32_t fd=default_fd;               //default fd
    dentry_t* thedentry;
    if (read_dentry_by_name(fname,thedentry)==-1) return -1;    // fail to find the name
    if (thedentry->filetype!=FILE_TYPE) return -1;              // this is not file
    my_file_table[fd].flags=1;                                  // 1 means in-use
    my_file_table[fd].inode=thedentry->inode;
    return 0;
}


/*
 * DESCRIPTION: 
 *          undo what I did in open, so just init file_table
 * INPUTS:  fd: the file descriptor, no use here since just use fd=2
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed
 * SIDE EFFECT: initialize the my_file_table[fd]
*/
int file_close(int32_t fd){
    init_file_table(fd);
    return 0;
}

/*
 * DESCRIPTION: 
 *          do nothing
 * INPUTS:  none
 * OUTPUTS: none
 * RETURN VALUE: -1
 * SIDE EFFECT: none
*/
int file_write(){
    return -1;
}


/*
 * DESCRIPTION: 
 *          use the fd to get the inode and position,
 *          read the content of the file by unit every
 *          time call this function, pass "unit" number
 *          of data into the buf
 * INPUTS:  fd: the descriptor
 *          unit: how many bytes you read every time
 *          buf: the buffer pointer, the destination
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, -1 if fail, 1 if come to the end
 * SIDE EFFECT: read and add the postion
*/
int file_read(int32_t fd, uint32_t unit, uint8_t* buf){
    int32_t result=0;
    uint32_t ino=my_file_table[fd].inode;
    uint32_t pos=my_file_table[fd].file_position;
    // printf("pos is:%d\n",pos);
    if (buf==NULL) return -1;
    result=read_data (ino, pos, buf, unit);
    if (result==-1) return -1;
    // printf("%s",buf);
    if (result==0) return 1;
    my_file_table[fd].file_position+=unit;
    return 0;
}


/*
 * DESCRIPTION: 
 *          then check if filename valid
 * INPUTS:  fname
 * OUTPUTS: none
 * RETURN VALUE: 0 if succeed, 1 if fail/invalid
 * SIDE EFFECT: none
*/
int directory_open(const uint8_t* fname){
    dentry_t* thedentry;
    if (read_dentry_by_name(fname,thedentry)==-1) return -1;    // fail to find the name
    if (thedentry->filetype!=1) return -1;                      // this is not directory
    return 0;
}

/*
 * DESCRIPTION: 
 *          do nothing
 * INPUTS:  none
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
*/
int directory_close(){
    return 0;
}

/*
 * DESCRIPTION: 
 *          do nothing
 * INPUTS:  none
 * OUTPUTS: none
 * RETURN VALUE: -1
 * SIDE EFFECT: none
*/
int directory_write(){
    return -1;
}

/*
 * DESCRIPTION: 
 *          printf all the filename included in the directory
 * INPUTS:  fname
 * OUTPUTS: none
 * RETURN VALUE: 0
 * SIDE EFFECT: none
*/
int directory_read(const uint8_t* fname){
    // printf("\ncome to dir_read  ");
    dentry_t* dentry_test;
	nodes_block* test_node;
	int32_t j;
	int32_t i;
	uint32_t helloinode;
	uint32_t filelength;
	uint32_t namelength;
	clear();
	for (j=0;j<=myboot->num_dir_entries-1;j++){
		read_dentry_by_index(j,dentry_test);
		helloinode=dentry_test->inode;
		test_node=(nodes_block*) (mynode+helloinode);
		filelength=test_node->length;
		
		namelength=strlen((int8_t*) dentry_test->filename);
		printf("file name :");
		// must use putc here, printf would like to find the '\0'
		for (i=0;i<NameLen;i++){
			putc(dentry_test->filename[i]);
		}
		//printf("       ");
		printf("file type:%d",dentry_test->filetype);

		printf(", file size: %d ",filelength);
		printf("\n");
	}
    return 0;
}





// useless functions
/*
boot_block* get_myboot(){
    return myboot;
}

 nodes_block* get_mynode(){
     return mynode;
 }

int test_file(){
    //test the address
        // int i;
        // printf("myboot information:  \n");
        // printf("   num_dir_entries: %d  \n",myboot->num_dir_entries);
        // printf("   num_inodes: %d  \n",myboot->num_inodes);
        // printf("   num_data_blocks: %d  \n",myboot->num_data_blocks);
        // for (i=0;i<myboot->num_dir_entries;i++){
        //     printf("   dentry_name[%d]:%s type:%d inode:%d     \n",i,myboot->dir_entries[i].filename, 
        //             myboot->dir_entries[i].filetype,myboot->dir_entries[i].inode);
        // }
    //test the read_dentry_by_name
        // dentry_t* dentry_test;
        // const char* s="verylargetextwithverylongname.txt";
        // read_dentry_by_name((const uint8_t*)s,dentry_test);
        // printf("file name:%s",dentry_test->filename);
    //test the read_dentry_by_index
        // dentry_t* dentry_test;
        // int j;
        // int i;
        // uint32_t helloinode;
        // nodes_block* test_node;
        // uint32_t filelength;
        // uint32_t namelength;
        // clear();
        // printf("\n\n\n");
        // for (j=0;j<=myboot->num_dir_entries-1;j++){
        //     read_dentry_by_index(j,dentry_test);
        //     helloinode=dentry_test->inode;
        //     test_node=(nodes_block*) (mynode+helloinode);
        //     filelength=test_node->length;
            
        //     namelength=strlen((int8_t*) dentry_test->filename);
        //     printf("file name :");
        //     // must use putc here, printf would like to find the '\0'
        //     for (i=0;i<FILELENGTH;i++){
        //         putc(dentry_test->filename[i]);
        //     }
        //     printf("       ");
        //     printf("file type:%d",dentry_test->filetype);

        //     printf(", file size: %d ",filelength);
        //     printf("\n");
        // }
    //test the read_data and offset
        // dentry_t test_file=myboot->dir_entries[15];
        // uint32_t helloinode=test_file.inode;
        // uint8_t* namestring=test_file.filename;
        // nodes_block* test_node=(nodes_block*) (mynode+helloinode);
        // uint32_t filelength=test_node->length;

        // uint8_t buffer[50000];              // if use pointer, we must set the last equal to "\0"
        // uint32_t offset=0;
        // int32_t test_length=filelength-offset;     
        // if (test_length<0) {
        //     printf("please change the offset");
        //     return -1;
        // }
        // // offset=2, filelength=10, data is 0\1\2\3\4\5\6\7\8\9
        // // offset=2 means start from 2, so length=8

        // clear();
        // printf("\n\n\n");
        // read_data (helloinode, offset, buffer, test_length);
        // printf("%s",buffer);
        // printf("\nfile name: %s", namestring);
        // printf("\nfile length is:%d\n",filelength);
    return 0;
}
*/

