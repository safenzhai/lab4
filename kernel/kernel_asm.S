@ kernel__asm.S: asm stuff for the kernel 
@ authors: Sridhar Srinivasan <sridhar1>
@          Ramya Bolla <rbolla>
@          Vinay Prasad <vinayp1>

#include <kernel_asm.h>
#include <config.h>
#include <arm/psr.h>

	.file	"kernel_asm.S"
	.text
	.align 2
	.global s_handler
	.global enter_user_mode
	.global get_kernel_sp

@ custom s_handler implementation
s_handler:
	sub sp, sp, #WORD_OFFSET      @ make space for storing spsr
	stmfd sp!, {r0-r12, lr}      
@	ldr r8, [sp, #R8_OFFSET]      @ restore kernel's copy of r8
    ldr r0, =global_data
    ldr r8, [r0]
@	mrs r0, cpsr                  @ enable interrupts here... 
@	bic r0, r0, #PSR_IRQ
@	msr cpsr, r0
@    bl enable_interrupts
	mrs r2, spsr                  
	str r2, [sp, #SPSR_STACK_OFFSET]  @ save spsr on the space created on stack
	mov r1, sp                    @ setup 2nd arg for C_SWI_Handler
	ldr r0, [lr, #-WORD_OFFSET] 
	bic r0, r0, #SWI_NUM_MASK     @ setup 1st arg for C_SWI_Handler

	stmfd sp, {sp, lr}^
	add sp, sp, #-8

	bl C_SWI_Handler              @hand off control to C_SWI_Handler

	ldmfd sp, {sp, lr}^
	add sp, sp, #8

	ldr r2, [sp, #SPSR_STACK_OFFSET]
	msr spsr, r2                  @ restore spsr
	ldmfd sp!, {r0-r12, lr}       @unwind stack and return to caller 
	add sp, sp, #WORD_OFFSET
	movs pc, lr

@utility function that launches a user task
enter_user_mode:
@	stmfd sp!, {r8}
@	stmfd sp!, {r0}
@	mov r0, lr
@	add r1, sp, #TWO_WORD_OFFSET
@	bl save_lr_and_sp_for_exit    @ save the sp and lr for use in exit handler
@	ldmfd sp!, {r0}               
	mov r3, #CPSR_USER_MODE
	orr r3, r3, #0x80
	mrs ip, cpsr                  @ change mode bits in CPSR to <flags>i_USER
	bic ip, ip, #CPSR_LAUNCH_APP_MASK
	orr ip, ip, r3
	msr cpsr, ip
@	mov sp, r0
	mov sp, #USR_STACK
	ldr pc, =USR_START_ADDR       @ move control to _start of user task

#if 0
@utility method for getting the value of r0 register
get_kernel_r0:
	mov pc, lr

@utility method to setup the IRQ mode stack bottom to sp_irq
setup_irq_stack:
	mov r1, #CPSR_IRQ_MODE
	mrs ip, cpsr                  @ change mode bits in CPSR to <flags>_IRQ
	bic ip, ip, #CPSR_MODE_MASK
	orr ip, ip, r1
	msr cpsr, ip                  @switch to IRQ mode
	mov sp, r0                    @point sp_irq to allocated irq_stack
	mov r1, #CPSR_SVC_MODE
	mrs ip, cpsr                  @ change mode bits in CPSR to <flags>_SVC
	bic ip, ip, #CPSR_MODE_MASK
	orr ip, ip, r1
	msr cpsr, ip                  @switch back to SVC mode
	mov pc, lr

disable_intr:
	mrs r0, cpsr                  @ disable interrupts here... 
	orr r0, r0, #IRQ_MASK
	msr cpsr, r0
	mov pc, lr
#endif

get_kernel_sp:
	mov r0, sp
	mov pc, lr
