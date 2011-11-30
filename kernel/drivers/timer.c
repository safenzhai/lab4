/*
 * timer.c: Implemetation of the timer driver 
 *
 * Authors: Sridhar Srinivasan <sridhar1@andrew.cmu.edu>
 *          Ramya Bolla <rbolla@andrew.cmu.edu>
 *          Vinay Prasad <vinayp1@andrew.cmu.edu>
 */

#include <arm/timer.h>
#include <types.h>
#include <arm/reg.h>
#include <exports.h>
#include <config.h>
#include <sched.h>
#include <device.h>

#define TIMER_FREQ_FACTOR 100

/*
 * globals
 */
volatile unsigned long num_ticks;
unsigned long overflow_count = 0;

void init_timer(void)
{
	uint32_t oscr_10ms = 0;
	uint32_t oier_reg;

	/*
	 * init the oscr with 0
	 */
	do {
		reg_write(OSTMR_OSCR_ADDR, 0x0);
	} while(reg_read(OSTMR_OSCR_ADDR) != 0);
		
	/*
	 * calculate the oscr value for a 10 ms duration
	 */
	oscr_10ms = OSTMR_FREQ/TIMER_FREQ_FACTOR;

	/*
	 * init the osmr0 reg to oscr10_ms
	 */
	reg_write(OSTMR_OSMR_ADDR(0), oscr_10ms);
	
	/*
	 * activate the osmr0 bit in oier
	 */
	oier_reg = reg_read(OSTMR_OIER_ADDR);
	oier_reg |= OSTMR_OIER_E0;
	reg_write(OSTMR_OIER_ADDR, oier_reg);
	
	/*
	 * all set!
	 */
	return;
}

void timer_handler(unsigned int int_num)
{		        
	uint32_t ossr_reg;

	/*
	 * increment the numticks
	 */
//	printf("inside timer handler\n");
	num_ticks++;
	if(num_ticks == 0) {
		// there is an overflow
		overflow_count++;
		printf("OVERFLOW IN NUM_TICKS. THE VALUE HAS WRAPPED AROUND %lu NO. OF TIMES\n", overflow_count);
	}
	/*
	 * reset the OSCR
	 */
//	do {
		reg_write(OSTMR_OSCR_ADDR, 0x0);
//	} while(reg_read(OSTMR_OSCR_ADDR) != 0);

	/*
	 * acknowlegde the interrupt
	 */
	ossr_reg = reg_read(OSTMR_OSSR_ADDR);
	ossr_reg |= OSTMR_OSSR_M0;
	reg_write(OSTMR_OSSR_ADDR, ossr_reg);

	/*
	 * update the devices
	 */
	dev_update(get_millis());

	/*
	 * perform context switch
	 */
//	if(!(num_ticks % 50)) {
	dispatch_save();
//	}

	int_num = int_num;
	return;
}

unsigned long get_ticks(void)
{
	return num_ticks;
}

unsigned long get_millis(void)
{
	return (get_ticks() * OS_TIMER_RESOLUTION); 
}

//TODO

void destroy_timer(void)
{
	return;
}
