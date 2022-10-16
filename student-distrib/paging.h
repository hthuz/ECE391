#ifndef PAGING_H
#define PAGING_H


#define PDE_NUM 1024
#define PTE_NUM 1024
#define KERNEL_ADDR (4*1024*1024)  //0x00400000 starting address of kernel memory


// struct of 4kB page directiry entry
j/ refer to IA32-ref-manual-vol-3 page 3-24
// size: 32bits
struct pde_t
{
  int present: 1;
  int r_w: 1;
  int u_su: 1;
  int write_through: 1;
  int cache_dis: 1;
  int accessed: 1;
  int reserved: 1;
  int page_size: 1;
  int global_page: 1;
  int avail: 3;
  int base_addr:20; 
};



// struct of 4kB page table entry
// refer to IA32-ref-manual-vol-3 page 3-24
// size: 32bits
struct pte_t
{
  int present: 1;
  int r_w: 1;
  int u_su: 1;
  int write_through: 1;
  int cache_dis: 1;
  int accessed: 1;
  int dirty: 1;
  int pat: 1; // Page Table Attribute Index
  int global_page: 1;
  int avail: 3;
  int base_addr:20;
};


typedef struct pde_t pde_t;
typedef struct pte_t pte_t;




#endif
