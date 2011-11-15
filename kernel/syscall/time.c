/** @file time.c Implementation of the time syscall 
 * Authors: Sridhar Srinivasan <sridhar1@andrew.cmu.edu>
 *          Ramya Bolla <rbolla@andrew.cmu.edu>
 *          Vinay Prasad <vinayp1@andrew.cmu.edu>
 *
 */
 

#include <types.h>
#include <config.h>
#include <bits/errno.h>
#include <arm/timer.h>
#include <syscall.h>


/*
 * implementation of the time syscall
 * @param: void 
 * @return unsigned long - time in milliseconds since bootup 
 */
unsigned long time_syscall(void)
{
	#if 0
 	return (get_ticks() * OS_TIMER_RESOLUTION);
	#endif
	return 0;
}

/*
 * implementation of the sleep syscall
 * @param: millis - number of milliseconds to sleep  
 * @return void 
 */
void sleep_syscall(unsigned long millis)
{
	#if 0
	unsigned long sleep_till_ticks;
	/*
	 * validate the millis arg
	 */
	if(millis == 0) {
		return;
	}

	/*
	 * calculate the number of ticks that should have
	 * elapsed before we wake up
	 */
	sleep_till_ticks = get_ticks() + (millis/OS_TIMER_RESOLUTION);
	while(get_ticks() < sleep_till_ticks);

	/*
	 * time to wake up!
	 */
	#endif
	millis = millis;
	return;
	
}
