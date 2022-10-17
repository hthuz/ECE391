/* paging.c - paging part 
*/



#include "paging.h"
#include "lib.h"
#include "types.h"

pde_t p_dir[PDE_NUM] __attribute__((aligned (P_4K_SIZE)));
pte_t p_table[PTE_NUM] __attribute__((aligned (P_4K_SIZE)));



/* 
 * paging_init
 * DESCRIPTION: initalize 4MB paging for kernel and 4KB for video memory
 * INPUTS: none
 * OUTPUTS: initialize paging
 * RETURN VALUE: 1 if paing initialization successes
 *               0 otherwise
 */
char paging_init()
{
  int i;

  // initalize directory
  for(i = 0; i < PDE_NUM; i++)
  {
    p_dir[i].present = 0;      // initially unpresent
    p_dir[i].r_w = 1;          // mark all pages read/write
    p_dir[i].u_su = 0;         // default supervisor only 
    p_dir[i].write_through = 0;// use write back, always 0
    p_dir[i].cache_dis = 0;    // default no cache
    p_dir[i].accessed = 0;     // unrelated
    p_dir[i].page_size = 0;    // default 4kB page
    p_dir[i].global_page = 0;  // default not global
    p_dir[i].avail = 0;        // unrelated
    p_dir[i].base_addr = 0;    // default 0
  }

  // initialize table for video memory
  for(i = 0; i < PTE_NUM; i++) 
  {
    p_table[i].present = 0;      // initially unpresent
    p_table[i].r_w = 1;          // mark all pages read/write
    p_table[i].u_su = 0;         // default supervisor only
    p_table[i].write_through = 0;// use write back, always 0
    p_table[i].cache_dis = 0;    // default no cache
    p_table[i].accessed = 0;     // unrelated
    p_table[i].dirty = 0;        // unrelated
    p_table[i].pat = 0;          // unrelated
    p_table[i].avail = 0;        // unrelated
    p_table[i].base_addr = 0;    // default 0
  }



  // paging for video memory
  // set up PTE for video memory
  // for(i = VID_MEM_START; i < VID_MEM_END; i = i + P_4K_SIZE )
  // PTE_INDEX(i) is always 0 for video memory so no loop required
  p_dir[0].present = 1;
  p_dir[0].base_addr = (((int)p_table) >> 12); // p_table is 4k aligned. the address is 
                                                          // bound to be a multiple of 4k(2^12)
                                                          // so lower 12 bits are not required

  // set up PDE for video memory
  for(i = VID_MEM_START; i < VID_MEM_END; i = i + P_4K_SIZE)
  {
    p_table[PTE_INDEX(i)].present = 1;
    p_table[PTE_INDEX(i)].cache_dis = 0; // cache_disable 0 for video memory
    p_table[PTE_INDEX(i)].base_addr = i >> 12;  // 12 is same reason as setting PTE
  }


  // paging for kernel memory
  // using 4MB paging
  p_dir[PDE_INDEX(P_4M_SIZE)].present = 1;
  p_dir[PDE_INDEX(P_4M_SIZE)].cache_dis = 1; // cache disbale for kernel
  p_dir[PDE_INDEX(P_4M_SIZE)].page_size = 1; // 4MB page size
  p_dir[PDE_INDEX(P_4M_SIZE)].global_page = 1;  // global for kernel
  p_dir[PDE_INDEX(P_4M_SIZE)].base_addr = P_4M_SIZE >> 12; // P_4M_SIZE is also start address
                                                           // of kernel memory

  return 1;
}

/*
 *
 *int main()
 *{
 *  int a;
 *  paging_init();
 *  for(a = 0; a < PTE_NUM; a++)
 *  {
 *    printf("%d, %d\n",a, p_table[a].base_addr);
 *  }
 *
 *
 *}
 */
