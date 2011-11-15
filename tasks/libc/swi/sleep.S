/*
 * sleep.S: sleep sycall wrapper
 * Author:  Sridhar  Srinivasan <sridhar1@andrew.cmu.edu>
 *         Vinay Prasad <vinayp1@andrew.cmu.edu>
 *         Ramya Bolla <rbolla@andrew.cmu.edu>
 * 
 */

#include <asm.h>
#include <bits/swi.h>

	.file "sleep.S"
FUNC(sleep)
sleep:
	swi SLEEP_SWI
	mov pc, lr
