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
#include <arm/psr.h>
#include <arm/exception.h>

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
	if((vector_addr != (unsigned int *)SWI_VECTOR_ADDR) 
	     && (vector_addr != (unsigned int *)IRQ_VECTOR_ADDR)) {
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
			enable_interrupts();
			r0 = *sp;
			r1 = *(sp + 1);
			r2 = *(sp + 2);
			*sp = read_syscall((int)r0, (void *)r1, (size_t)r2);
			break;
		case WRITE_SWI:
			enable_interrupts();
			r0 = *sp;
			r1 = *(sp + 1);
			r2 = *(sp + 2);
			*sp = write_syscall((int)r0, (const void *)r1, (size_t)r2);
			break;
		case TIME_SWI:
			enable_interrupts();
			*sp = time_syscall();
			break;
		case SLEEP_SWI:
//			enable_interrupts();
			r0 = *sp;
		    sleep_syscall((unsigned long)r0);
			break;
		case CREATE_SWI:
			r0 = *sp;
			r1 = *(sp + 1);
			*sp = task_create((task_t *)r0, (size_t)r1);
		break;
		case EVENT_WAIT:
			printf("calling event_wait sp is %p\n", sp);
			r0 = *sp;
			*sp = event_wait((unsigned int)r0);
//			while(1);	
			printf("returned from event_wait\n");
		break;
		case MUTEX_CREATE:
			*sp = mutex_create();
		break;	
		case MUTEX_LOCK:
			r0 = *sp;
			*sp = mutex_lock((int)r0);
		break;	
		case MUTEX_UNLOCK:
			r0 = *sp;
			*sp = mutex_unlock((int)r0);
		break;	
		default:
		    printf("\n C_SWI_Handler:invalid SWI call, panic\n");
			invalid_syscall(swi_num);	
	}
	return;
}

/*
 * implementation of the C_IRQ_Handler
 * @param: void
 * @return: void 
 */
void irq_handler(void) 
{
	uint32_t icpr_reg, osmr0_mask, ossr_reg;
//	printf("inside irq handler\n");
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
	timer_handler(icpr_reg);

	 /*
	  * acknowlegde the timer IRQ
//	do {
//		printf("inside do while loop\n");
		ossr_reg = reg_read(OSTMR_OSSR_ADDR);
		ossr_reg |= OSTMR_OSSR_M0;
		reg_write(OSTMR_OSSR_ADDR, ossr_reg);
//		ossr_reg &= ~OSTMR_OSSR_M0;
//	} while(reg_read(OSTMR_OSSR_ADDR) != ossr_reg);
	  */
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
