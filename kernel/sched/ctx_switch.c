/** @file ctx_switch.c
 * 
 * @brief C wrappers around assembly context switch routines.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-21
 */
 

#include <types.h>
#include <assert.h>

#include <config.h>
#include <kernel.h>
#include "sched_i.h"
#include <task.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <kernel_asm.h>
#ifdef DEBUG_MUTEX
#include <exports.h>
#endif

static tcb_t* cur_tcb; /* use this if needed */
unsigned int kstack_high_offset;
unsigned int ctx_lr_offset;
unsigned int get_kernel_sp(void);
/**
 * @brief Initialize the current TCB and priority.
 *
 * Set the initialization thread's priority to IDLE so that anything
 * will preempt it when dispatching the first task.
 */
void dispatch_init(tcb_t* idle)
{
	unsigned int *p = 0xA2FFFAF0;
	*p = 0;
	/*
	 * compute the offset form the context to the kstack_high
	 * in a tcb for future use
	 */
	kstack_high_offset = offsetof(tcb_t, kstack_high) - offsetof(tcb_t, context);
//	kstack_high_offset = 4148;
//	printf("offsetof(tcb_t, kstack_high) is %u\n", offsetof(tcb_t, kstack_high));
//	printf("offsetof(tcb_t, context) is %u\n", offsetof(tcb_t, context));
//	printf("dispatch_init kstack_high_offset is %u\n", kstack_high_offset);

	ctx_lr_offset = offsetof(sched_context_t, lr);
//	printf("offsetof(context, lr) is %u\n", ctx_lr_offset);
	// call find_next to find next task
//	cur_tcb = idle;
//	printf("inside dispatch init, calling ctx sw half\n");
//	printf("calling ctx swi half  ptr is %p\n", &(idle->context));
//	ctx_switch_half((volatile void *)(&(idle->context)));
	cur_tcb = NULL;	
}


/**
 * @brief Context switch to the highest priority task while saving off the 
 * current task state.
 *
 * This function needs to beexternally synchronized.
 * We could be switching from the idle task.  The priority searcher has been tuned
 * to return IDLE_PRIO for a completely empty run_queue case.
 */
void dispatch_save(void)
{
	uint8_t next_prio;
	tcb_t *next_tcb, *saved_cur_tcb;

//	printf("inside dispatch save\n");

//	printf("added cur_tcb %u %p to run queue\n", cur_tcb->cur_prio, cur_tcb);
	next_prio = highest_prio();
	/*
	 * add the current task to the run queue...
	 */
	runqueue_add(cur_tcb, cur_tcb->cur_prio);
//	printf("next_prio is %u\n", next_prio);
		

	next_tcb = runqueue_remove(next_prio);
//	printf(" d save: removed next_tcb %u %p from run queue\n", next_tcb->cur_prio, next_tcb);
//	print_run_queue();
	saved_cur_tcb = cur_tcb;
	cur_tcb = next_tcb;
#if 0
	printf("before calling ctx sw full, cur->context is %p\n", &(saved_cur_tcb->context));
	printf("hexdump of cur->context is\n");
	hexdump(&saved_cur_tcb->context, 160);
	printf("before calling ctx sw full, next->context is %p\n", &(next_tcb->context));
	printf("hexdump of next->context is\n");
	hexdump(&next_tcb->context, 160);
#endif
//	disable_interrupts();
	ctx_switch_full((volatile void *)(&(next_tcb->context)),
					(volatile void *)(&(saved_cur_tcb->context)));
//	while(1);
}

/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * don't save the current task state.
 *
 * There is always an idle task to switch to.
 */
void dispatch_nosave(void)
{
	uint8_t next_prio;
	tcb_t *next_tcb;
//	printf("inside dispatch no save\n");
	next_prio = highest_prio();
//	printf("next_prio is %u\n", next_prio);
		
	/*
	 * manage the run queue...
	 */
	next_tcb = runqueue_remove(next_prio);
//	printf("d nosave: removed next_tcb %u %p from run queue\n", next_tcb->cur_prio, next_tcb);
//	print_run_queue();
	cur_tcb = next_tcb;
//	printf("before calling ctx sw half, context->sp is %p\n", next_tcb->context.sp);
	ctx_switch_half((volatile void *)(&(next_tcb->context)));
}


/**
 * @brief Context switch to the highest priority task that is not this task -- 
 * and save the current task but don't mark is runnable.
 *
 * There is always an idle task to switch to.
 */
void dispatch_sleep(void)
{
	uint8_t next_prio;
	tcb_t *next_tcb, *saved_cur_tcb;

//	printf("inside dispatch sleep\n");
	next_prio = highest_prio();
//	printf("next_prio is %u\n", next_prio);
		
	next_tcb = runqueue_remove(next_prio);
//	printf("d sleep: removed next_tcb %u %p from run queue\n", next_tcb->cur_prio, next_tcb);
//	print_run_queue();
	saved_cur_tcb = cur_tcb;
	cur_tcb = next_tcb;
//	printf("before calling ctx sw full, next->context->sp is %p\n", next_tcb->context.sp);
//	printf("before calling ctx sw full, cur->context->sp is %p\n", saved_cur_tcb->context.sp);
//	printf("in dispatch sleep, sp is %u\n", get_kernel_sp());
//	hexdump(get_kernel_sp(), 200);
//	disable_interrupts();
//	printf("inside dispatch sleep hexdump of cur->context is\n");
//	hexdump(&saved_cur_tcb->context, 160);
	ctx_switch_full((volatile void *)(&(next_tcb->context)),
					(volatile void *)(&(saved_cur_tcb->context)));
//	while(1);
	
}

/**
 * @brief Returns the priority value of the current task.
 */
uint8_t get_cur_prio(void)
{
	return cur_tcb->cur_prio;
}

/**
 * @brief Returns the TCB of the current task.
 */
tcb_t* get_cur_tcb(void)
{
	return cur_tcb;	
}

void debug_print(void)
{
	unsigned int *p = 0xA2FFFAF0;
	*p += 1;
}
