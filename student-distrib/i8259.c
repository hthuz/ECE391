
/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask=maskall; /* IRQs 0-7  */
uint8_t slave_mask=maskall;  /* IRQs 8-15 */


/* 
 * initialize_keyboard
 * DESCRIPTION:  Initialize the 8259 PIC 
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: initialize the i8259.
 */
void i8259_init(void) {

    outb(maskall,MASTER_8259_DATA);
    outb(maskall,SLAVE_8259_DATA);
/* Master */
    outb(ICW1, MASTER_8259_PORT);		   
	outb(ICW2_MASTER, MASTER_8259_DATA);	
	outb(ICW3_MASTER, MASTER_8259_DATA);    
	outb(ICW4, MASTER_8259_DATA);		   
/* Slave */
	outb(ICW1, SLAVE_8259_PORT);		
	outb(ICW2_SLAVE, SLAVE_8259_DATA);	
	outb(ICW3_SLAVE, SLAVE_8259_DATA);	
	outb(ICW4 , SLAVE_8259_DATA);	

    outb(master_mask,MASTER_8259_DATA);       
    outb(slave_mask, SLAVE_8259_DATA);
    enable_irq(2);  // open the master irq2 so that the slave could work
}


/* 
 * enable_irq
 * DESCRIPTION:  Enable (unmask) the specified IRQ 
 * INPUTS: irq_num: 0-15 number , represent IRQ 0-15
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: have the new changed master_mask or new changed slave_mask updated
 */
void enable_irq(uint32_t irq_num) {
    if (!(irq_num & eight)){  // master
        uint8_t mymask= ( 0x80 >>(7-irq_num) ); //0x80=1000 0000,magic number 7 for calculation of master
        mymask=~mymask;
        master_mask= ( master_mask & mymask );
        outb(master_mask,MASTER_8259_DATA);
    }
    else if (irq_num & eight){  //slave
        master_mask= ( master_mask | 0x02) ;
        outb(master_mask,MASTER_8259_DATA);

        uint8_t mymask= ( 0x80 >>(15-irq_num) ); //0x80=1000 0000,magic number 15 for calculation of slave
        mymask=~mymask;
        slave_mask= ( slave_mask & mymask );
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* 
 * disable_irq
 * DESCRIPTION:  disable (mask) the specified IRQ 
 * INPUTS: irq_num: 0-15 number , represent IRQ 0-15
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: have the new changed master_mask or new changed slave_mask updated
 */
void disable_irq(uint32_t irq_num) {
    if (!(irq_num & eight)){    //master
        uint8_t mymask= ( 0x80>>(7-irq_num) );  //0x80=1000 0000,magic number 7 for calculation of master
        master_mask= ( master_mask | mymask );
        outb(master_mask,MASTER_8259_DATA);
    }
    else if (irq_num & eight){  //slave

        uint8_t mymask= ( 0x80>>(15-irq_num) ); //0x80=1000 0000,magic number 15 for calculation of slave
        slave_mask= ( slave_mask | mymask );
        outb(slave_mask,SLAVE_8259_DATA);
    }
}

/* 
 * send_eoi
 * DESCRIPTION:  send eoi
 * INPUTS: irq_num: 0-15 number , represent IRQ 0-15
 * OUTPUTS: none
 * RETURN VALUE: none
 * SIDE EFFECT: give the pic the signal that the interrupt is finished
 */
void send_eoi(uint32_t irq_num) {
    if (irq_num & eight) {      //slave
		outb(EOI|(irq_num & 7),SLAVE_8259_PORT);    // get the last three bits, to slave
		outb(EOI|2,MASTER_8259_PORT);	            // IRQ2
	} 
    else {        //master
		outb(EOI+irq_num,MASTER_8259_PORT);	        // just set master
	}
}

/* 
 * get_master/slave_mask
 * DESCRIPTION:  get the local variable to test
 * INPUTS: none
 * OUTPUTS: none
 * RETURN VALUE: master_mask/slave_mask
 * SIDE EFFECT: none
 */
uint8_t get_master_mask(){
    return master_mask;
}
uint8_t get_slave_mask(){
    return slave_mask;
}
