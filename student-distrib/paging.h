#ifndef PAGING_H
#define PAGING_H


#define PDE_NUM 1024
#define PTE_NUM 1024
#define VID_MEM_START 0x000B8000
#define VID_MEM_END 0x000C0000
#define P_4K_SIZE (4*1024)
#define P_4M_SIZE (1024 * P_4K_SIZE)
#define P_128M_SIZE (32 * P_4M_SIZE)
#define KERNEL_ADDR (4*1024*1024)  //0x00400000 starting address of kernel memory

#define PDE_INDEX(vir_mem) (( (vir_mem) & 0xFFC00000) >> 22)
#define PTE_INDEX(vir_mem)  ( ((vir_mem) & 0x003FF000) >> 12)
#define P_OFFSET(vir_mem) ( (vir_mem) & 0x00000FFF)


// struct of 4kB page directiry entry
// refer to IA32-ref-manual-vol-3 page 3-24
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

extern pde_t p_dir[PDE_NUM] __attribute__((aligned (P_4K_SIZE)));
extern pte_t p_table[PTE_NUM] __attribute__((aligned (P_4K_SIZE)));
extern pte_t video_p_table[PTE_NUM] __attribute__((aligned (P_4K_SIZE)));

char paging_init();

void flush_tlb();
#endif

