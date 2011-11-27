/** @file run_queue.c
 * 
 * @brief Run queue maintainence routines.
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-21
 */

#include <types.h>
#include <assert.h>

#include <kernel.h>
#include <sched.h>
#include "sched_i.h"

#define GROUP_SHIFT 3
#define TASK_POSITION_MASK 0x7

static tcb_t* run_list[OS_MAX_TASKS];

/* A high bit in this bitmap means that the task whose priority is
 * equal to the bit number of the high bit is runnable.
 */
static uint8_t run_bits[OS_MAX_TASKS/8];

/* This is a trie structure.  Tasks are grouped in groups of 8.  If any task
 * in a particular group is runnable, the corresponding group flag is set.
 * Since we can only have 64 possible tasks, a single byte can represent the
 * run bits of all 8 groups.
 */
static uint8_t group_run_bits;

/* This unmap table finds the bit position of the lowest bit in a given byte
 * Useful for doing reverse lookup.
 */
static uint8_t prio_unmap_table[] =
{

0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};

/**
 * @brief Clears the run-queues and sets them all to empty.
 */
void runqueue_init(void)
{
	int i;
	group_run_bits = 0;
	for(i = 0; i < OS_MAX_TASKS; i++) {
		run_list[i] = NULL;
	}
	for(i = 0; i < OS_MAX_TASKS/8; i++) {
		run_bits[i] = 0;
	}
}

/**
 * @brief Adds the thread identified by the given TCB to the runqueue at
 * a given priority.
 *
 * The native priority of the thread need not be the specified priority.  The
 * only requirement is that the run queue for that priority is empty.  This
 * function needs to be externally synchronized.
 */
void runqueue_add(tcb_t* tcb, uint8_t prio)
{
	uint8_t ostcbx, ostcby;
	// add to run list
	run_list[prio] = tcb;

	// add to group_run_bits
	ostcby = prio >> GROUP_SHIFT;
	group_run_bits |= (0x1 << ostcby);

	// add to the run_bits
	ostcbx = prio & TASK_POSITION_MASK;
	run_bits[ostcby] |= (0x1 << ostcbx); 	
}


/**
 * @brief Empty the run queue of the given priority.
 *
 * @return  The tcb at enqueued at the given priority.
 *
 * This function needs to be externally synchronized.
 */
tcb_t* runqueue_remove(uint8_t prio)
{
	uint8_t ostcbx, ostcby;
	tcb_t *return_tcb = NULL;

	// remove from run list
	return_tcb = run_list[prio];
	run_list[prio] = NULL;


	ostcby = prio >> GROUP_SHIFT;

	// remove_from the run_bits
	ostcbx = prio & TASK_POSITION_MASK;
	run_bits[ostcby] &= ~(0x1 << ostcbx); 	
		
	// remove from group_run_bits if applicable
	if(run_bits[ostcby] == 0) {
		group_run_bits &= ~(0x1 << ostcby);
	}
	return return_tcb;
}

/**
 * @brief This function examines the run bits and the run queue and returns the
 * priority of the runnable task with the highest priority (lower number).
 */
uint8_t highest_prio(void)
{
	uint8_t x, y, prio;

	y = prio_unmap_table[group_run_bits];
	x = prio_unmap_table[run_bits[y]];

	prio = (y << GROUP_SHIFT) + x;

	if((prio == 0) && (group_run_bits == 0)) {
		return IDLE_PRIO;
	}
	return prio;
}

void print_run_queue()
{
	int i;
	printf("RUN LIST\n");
	for(i = 0; i < OS_MAX_TASKS; i++) {
		printf("run_list[%d] = %p\n", i, run_list[i]);
	}
	printf("RUN BITS\n");
	for(i = 0; i < OS_MAX_TASKS/8; i++) {
		printf("run_bits[%d] = %x\n", i, run_bits[i]);
	}
	printf("GROUP_RUN_BITS = %x\n", group_run_bits);
}
