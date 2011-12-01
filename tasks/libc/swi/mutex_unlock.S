/** @file time.S
 *
 * @brief time sycall wrapper
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-10-31
 */

#include <asm.h>
#include <bits/swi.h>

	.file "mutex_unlock.S"

FUNC(mutex_unlock)
    swi MUTEX_UNLOCK
	cmp r0, #0
	movge pc, lr
	rsb r1, r0, #0
	ldr r2, =errno
	str r1, [r2]     
	mov r0, #-1          
	mov pc, lr
