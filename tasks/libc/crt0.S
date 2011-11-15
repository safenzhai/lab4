/*
 * crt0.S: Execution startup routine
 * Author:  Sridhar  Srinivasan <sridhar1@andrew.cmu.edu>
 *         Vinay Prasad <vinayp1@andrew.cmu.edu>
 *         Ramya Bolla <rbolla@andrew.cmu.edu>
 * 
 */

#include <asm.h>
#include <bits/swi.h>

	.file "crt0.S"

FUNC(_start)
_start:
    mov fp, #0
    mov lr, #0
    ldr r0, [sp]
	add r1, sp, #4
	bl main
loop:
	b loop
