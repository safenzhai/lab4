/** @file ctx_switch_asm.S
 *
 * @brief Contains the core context switch routine.
 *
 * These routines all assume that the caller is following AAPCS, needs
 * no co-processor registers and that the caller is in SVC mode. Furthermore,
 * the SPSR is considered a spare/scratch register and hence, is not saved.
 *
 * @author Kartik Subramanian
 * @date   2008-11-21
 */

	.file "ctx_switch_asm.S"

#include <asm.h>
#include <arm/psr.h>

/**
 * @brief Special exit routine from the scheduler that launches a task for the
 * first time.
 *
 * r4 contains the user entry point.
 * r5 contains the single argument to the user function called.
 * r6 contains the user-mode stack pointer.
 * Upon completion, we should be in user mode.
 */
FUNC(launch_task)
	mov     r0, r5
	mov     r1, #0
	mov     r2, #0
	mov     r3, #0
	mov     r5, #0
	mov     r8, #0
	mov     ip, #0
	msr     cpsr_c, #(PSR_MODE_USR)
@	msr     cpsr_c, #(PSR_MODE_USR | PSR_IRQ)
	mov     sp, r6
	mov     r6, #0
	ldr     lr, =0xdeadbeef   /* Causes a crash instead of calling the reset vector */
	mov     pc, r4

/* r0 points to the target context, r1 to the current context. */
/* add your code to perform a full context switch */
FUNC(ctx_switch_full)
	stmia r1, {r4, r5, r6, r7, r8, r9, r10, r11, sp, lr}
@	stmfd sp!, {r0, r1}
@	mov r0, r1
@	mov r1, #160
@	bl hexdump
@	ldmfd sp!, {r0, r1}
@loop: b loop
@	mov ip, r0
@	mov r0,ip 
@	mov r1, #160
@	bl hexdump
@	ldr r1, =ctx_lr_offset
@	ldr r2, [r1]
@	ldr r3, [r0, #36]
@	ldr r1, =0xcafebabe
@	ldr r1, =kstack_high_offset
@	ldr r2, [r1]
@	add r1, r0, r2
@	ldr r2 , =0xA2FFFAF0
@	str r3, [r2]
@	str r1, [r2, #4]
@	cmp r3, r1
@	bne already_launched
@	bl debug_print
@	bl ctx_switch_half
@already_launched:
	ldmia r0, {r4, r5, r6, r7, r8, r9, r10, r11, sp, pc}
@	mov pc, lr

/*add your code to perform a half context switch */	
FUNC(ctx_switch_half)
	ldr r4, [r0]
	ldr r5, [r0, #4]
	ldr r6, [r0, #8]
	ldr sp, [r0, #32]
	ldr pc, [r0, #36]
@	b launch_task
