/** @file sched.c
 * 
 * @brief Top level implementation of the scheduler.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-20
 */

#include <types.h>
#include <assert.h>

#include <kernel.h>
#include <config.h>
#include "sched_i.h"
#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>

// TODO: REMOVE THIS
#include <arm/timer.h>
tcb_t system_tcb[OS_MAX_TASKS]; /*allocate memory for system TCBs */
uint32_t *idle_count = 0xa2fffaf0;

/**
 * @brief This is the idle task that the system runs when no other task is runnable
 */
 
static void idle(void) 
{
	*idle_count = *idle_count + 1;
	enable_interrupts();
	while(1);
}


void setup_task_context(task_t *task, tcb_t *tcb, uint8_t prio)
{
    sched_context_t *context = &(tcb->context);
	tcb->native_prio = prio;
	tcb->cur_prio = prio;
	printf("setting up context with %p %p %p\n", task->lambda, task->data, task->stack_pos);
	context->r4 = (uint32_t)task->lambda;
	context->r5 = (uint32_t)task->data;
	context->r6 = (uint32_t)task->stack_pos;
	context->sp = (void *)(tcb->kstack_high);
	printf("after setting up context %u %u %u sp is %u \n", (tcb->context).r4, (tcb->context).r5, (tcb->context).r6, (tcb->context).sp);
	printf("tcb->kstack_high is %x\n", (uint32_t)tcb->kstack_high);
	printf("&(tcb->kstack_high) is %x\n", (uint32_t)(&(tcb->kstack_high)));
	tcb->holds_lock = 0;
	tcb->sleep_queue = NULL;
}

void sched_init(task_t* main_task)
{
	*idle_count = 0;
	main_task->lambda = (task_fun_t)idle;
	main_task->data = NULL;
	main_task->stack_pos = system_tcb[IDLE_PRIO].kstack_high;
	main_task->C = 0;
	main_task->T = 0;
	printf("setting stuff in idle's tcb %p\n", &system_tcb[IDLE_PRIO]);
	// setup the context for the idle task
	setup_task_context(main_task, &system_tcb[IDLE_PRIO], IDLE_PRIO);
	runqueue_add(&system_tcb[IDLE_PRIO], IDLE_PRIO);
}

/**
 * @brief Allocate user-stacks and initializes the kernel contexts of the
 * given threads.
 *
 * This function assumes that:
 * - num_tasks < number of tasks allowed on the system.
 * - the tasks have already been deemed schedulable and have been appropriately
 *   scheduled.  In particular, this means that the task list is sorted in order
 *   of priority -- higher priority tasks come first.
 *
 * @param tasks  A list of scheduled task descriptors.
 * @param size   The number of tasks is the list.
 */
void allocate_tasks(task_t** tasks, size_t num_tasks)
{
	task_t *a_tasks = *tasks;
	unsigned int i;
	for(i = 0; i < num_tasks; i++) {
		setup_task_context(&a_tasks[i], &system_tcb[i+1], i+1);
	}
}
