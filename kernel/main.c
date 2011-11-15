/** @file main.c
 *
 * @brief kernel main
 *
 * @author 
 *	   
 *	   
 * @date   
 */
 
#include <kernel.h>
#include <task.h>
#include <sched.h>
#include <device.h>
#include <assert.h>
#include "handlers.h"

uint32_t global_data;

int kmain(int argc, char** argv, uint32_t table)
{

	app_startup();
	global_data = table;
	/* add your code up to assert statement */
	
	/*
	 * set up the custom swi ad irq handler by hijacking 
	 * the existing swi handling
	 * infrastructure
	 */
	if(install_handler((unsigned int *)SWI_VECTOR_ADDR, (void *)s_handler) < 0){
		printf("\n KERNEL MAIN: installation of custom SWI handler failed");
		return 0xbadc0de;
}
#if 0	
	if(install_handler((unsigned int *)IRQ_VECTOR_ADDR, (void *)i_handler) < 0){
		printf("\n KERNEL MAIN: installation of custom IRQ handler failed");
		return 0xbadc0de;
}

/*
	 * init the IRQ related registers
	 */
	init_irq_regs();	
	
	/*
	 * setup IRQ stack
	 */
	setup_irq_stack(irq_stack + IRQ_STACK_SIZE - sizeof(long));

	/*
	 * init the timer driver
	 */
	init_timer_driver();

#endif
	/*
	 * launch the user task
	 */
	enter_user_mode();	
	argc = argc;
	argv[0] = argv[0];

	assert(0);        /* should never get here */
}
