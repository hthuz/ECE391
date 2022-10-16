/* paging.c - paging part 
*/



#include "paging.h"
#include "lib.h"

pde_t p_dir[PDE_NUM];
pte_t p_table[PTE_NUM];



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
  // initalize, not present in physical memory first
  for(i = 0; i < PDE_NUM; i++)
  {
    p_dir[i].present = 0;
    p_table[i].present = 0;
  }

  p_dir[0].present = 1;
  p_dir[0].base_addr = (int)p_table >> 12;

  p_dir[1].p = 1;
  p_dir[1].page_size = 1;
  p_dir[1].base_addr = 0x400;







  return 1;
}


int main()
{

  printf("%x\n",KERNEL_ADDR);

}
