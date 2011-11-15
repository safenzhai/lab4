/*
 * handlers.c: wire-in and hijacking code for SWI and IRQ handlers
 *
 * Authors: Sridhar Srinivasan <sridhar1@andrew.cmu.edu>
 *          Ramya Bolla <rbolla@andrew.cmu.edu>
 *          Vinay Prasad <vinayp1@andrew.cmu.edu>
 */
#include "handlers.h"
#include <exports.h>
#include <bits/swi.h>
#include <arm/interrupt.h>
#include <arm/timer.h>
#include <arm/reg.h>
#include <syscall.h>

/*
 * globals
 */
#if 0
unsigned int def_swi_handler_inst1, def_swi_handler_inst2;
unsigned int def_irq_handler_inst1, def_irq_handler_inst2;
#endif
unsigned int *def_swi_handler_loc;
//unsigned int *def_irq_handler_loc;


 /*
 * function that installs the custom handler by hijacking the first 2 
 * instructions of the default handler
 * @param: void
 * @return: 0 on success, < 0 on failure
 */
int install_handler(unsigned int *vector_addr, void *handler_addr)
{
	unsigned int vector_inst;
	unsigned int offset;
	unsigned int *def_handler_loc;

	/*
	 * validate the vector address
	 */
	if((vector_addr != (unsigned int *)SWI_VECTOR_ADDR)) {
//	&&
//	   (vector_addr != (unsigned int *)IRQ_VECTOR_ADDR)) {
		printf("Invalid vector address passed to install_handler\n");
		return -1;
	}
	vector_inst = *vector_addr;
	offset = vector_inst & LDR_INST_OFFSET;
	/*
	 * validate the LDR instruction stored at SWI vector
	 */
	if((vector_inst & ~LDR_INST_OFFSET) != LDR_VALID_FORMAT) {
		printf("\nInvalid LDR instruction at vector,can't install handler");
		return -1;
	}
	
	/*
	 * fetch the offset from the LDR inst and go to the default handler
	 */
	def_handler_loc = (unsigned int *)(*(unsigned int *)((char *)vector_addr + 
												 ACTUAL_PC_OFFSET + offset));
#if 0
	/*
	 * save the existing instructions in the default handler
	 */
	switch ((unsigned int)vector_addr) {
		case SWI_VECTOR_ADDR:
			def_swi_handler_inst1 = *def_handler_loc;
			def_swi_handler_inst2 = *(def_handler_loc +1);
			def_swi_handler_loc = def_handler_loc;
			break;
		case IRQ_VECTOR_ADDR:
			def_irq_handler_inst1 = *def_handler_loc;
			def_irq_handler_inst2 = *(def_handler_loc +1);
			def_irq_handler_loc = def_handler_loc;
			break;		
		}
#endif 	
	/*
	 * overwrite the 1st 2 instructions of the default handler
	 */
	*def_handler_loc = (unsigned int)CUSTOM_HANDLER_INST1;
	*(def_handler_loc + 1) = (unsigned int)handler_addr;

	/*
	 * all set!
	 */
	return 0;
}

/*
 * implementation of the C_SWI_Handler
 * @param: swi_num- swi number
 * @param: sp- stack pointer that points to block of user registers
 * @return: return value form swi on success, -1 on failure (swi num not 
            supported)
 */
void C_SWI_Handler(int swi_num, unsigned int *sp)
{
	unsigned int r0, r1, r2;
	switch(swi_num) {
		case READ_SWI:
			r0 = *sp;
			r1 = *(sp + 1);
			r2 = *(sp + 2);
			*sp = read_syscall((int)r0, (void *)r1, (size_t)r2);
			break;
		case WRITE_SWI:
			r0 = *sp;
			r1 = *(sp + 1);
			r2 = *(sp + 2);
			*sp = write_syscall((int)r0, (const void *)r1, (size_t)r2);
			break;
		case TIME_SWI:
			*sp = time_syscall();
			break;
		case SLEEP_SWI:
			r0 = *sp;
		    sleep_syscall((unsigned long)r0);
			break;
#if 0
		case EXIT_SWI:
			/*
			 * restore the original s_handler and i_handler before dying 
			 */
			*def_swi_handler_loc = def_swi_handler_inst1;
			*(def_swi_handler_loc + 1) = def_swi_handler_inst2;
			*def_irq_handler_loc = def_irq_handler_inst1;
			*(def_irq_handler_loc + 1) = def_irq_handler_inst2;
			/*
			 * disable interrupts before handing over control to uboot
			 */
			disable_intr();
			r0 = *sp;
			kexit((int)r0);
			break;
#endif
		default:
		    printf("\n C_SWI_Handler:invalid SWI call,bail out with error\n");
			/*
			 * invalid swi call, bail out after marking failure 
			 * restore the original s_handler before dying 
			 
			*def_swi_handler_loc = def_swi_handler_inst1;
			*(def_swi_handler_loc + 1) = def_swi_handler_inst2;
			*def_irq_handler_loc = def_irq_handler_inst1;
			*(def_irq_handler_loc + 1) = def_irq_handler_inst2;
			kexit(0xbadc0de);
			*/
			invalid_syscall(swi_num);	
	}
	return;
}
#if 0
/*
 * implementation of the C_IRQ_Handler
 * @param: void
 * @return: void 
 */
void C_IRQ_Handler(void) 
{
	uint32_t icpr_reg, osmr0_mask, ossr_reg;
	/*
	 * identify the source of IRQ
	 */
	icpr_reg = reg_read(INT_ICIP_ADDR);

	/*
	 * if the source is not osmr0 == oscr, bail out
	 */
	osmr0_mask = 0x1 << INT_OSTMR_0;
	if(!(icpr_reg & osmr0_mask)) {
		printf("\n C_IRQ_Handler, IRQ from unsupported source, bailing out\n");
		return;
	}

	/*
	 * redirect control to timer handler
	 */
	 handle_timer_irq();

	 /*
	  * acknowlegde the timer IRQ
	  */
	ossr_reg = reg_read(OSTMR_OSSR_ADDR);
	ossr_reg |= OSTMR_OSSR_M0;
	reg_write(OSTMR_OSSR_ADDR, ossr_reg);
	ossr_reg = reg_read(OSTMR_OSSR_ADDR);
	return;
}

/*
 * function to initialize the IRQ controller registers
 * @param void
 * @return void
 */
void init_irq_regs(void)
{
	uint32_t icmr_mask, iclr_reg, iclr_mask;
	icmr_mask = (0x1 << INT_OSTMR_0);
	
	/*
	 * write this mask into ICMR
	 */
	reg_write(INT_ICMR_ADDR, icmr_mask);

	/*
	 * ensure that the OSSRMR0 interrupt is routed as an IRQ
	 */
	iclr_reg = reg_read(INT_ICLR_ADDR);
	iclr_mask = ~(0x1 << INT_OSTMR_0);
	iclr_reg &= iclr_mask;
	reg_write(INT_ICLR_ADDR, iclr_reg);
	
	return;
}
#endif
