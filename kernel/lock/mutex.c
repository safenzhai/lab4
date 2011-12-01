/**
 * @file mutex.c
 *
 * @brief Implements mutices.
 *
 * @author Harry Q Bovik < PUT YOUR NAMES HERE
 *
 * 
 * @date  
 */

#define DEBUG_MUTEX

#include <lock.h>
#include <task.h>
#include <sched.h>
#include <bits/errno.h>
#include <arm/psr.h>
#include <arm/exception.h>
#ifdef DEBUG_MUTEX
#include <exports.h> // temp
#endif
#include <types.h>

mutex_t gtMutex[OS_NUM_MUTEX];

void mutex_init()
{
	int i;
	/*
	 * init all the mutex to default values
	 */
	for(i = 0; i < OS_NUM_MUTEX; i++) {
		gtMutex[i].bAvailable = TRUE;
		gtMutex[i].pHolding_Tcb = NULL;
		gtMutex[i].bLock = FALSE;
		gtMutex[i].pSleep_queue = NULL;
	}
}

int mutex_create(void)
{
	int i;

	/*
	 * find the next available mutex
	 */
	for(i = 0; i < OS_NUM_MUTEX; i++) {
		if(gtMutex[i].bAvailable == TRUE) break;
	}

	if(i == OS_NUM_MUTEX) {
		printf("no mutexes available\n");
		return -ENOMEM;
	}

	/*
	 * mark this mutex as not available
	 */
	gtMutex[i].bAvailable = FALSE;
	return i;
}

void add_to_mutex_sleep_queue(mutex_t *mut, tcb_t *target_tcb)
{
	tcb_t *prev_tcb = NULL;
	tcb_t *cur_tcb;

//	printf("adding %d to sleep queue head is \n", target_tcb->native_prio, 
//	                                              mut->pSleep_queue);
	
	if(mut->pSleep_queue == NULL) {
		mut->pSleep_queue = target_tcb;	
		target_tcb->sleep_queue = NULL;
//		printf("inside add finction, after adding, inside if head is %p\n", 
//		    mut->pSleep_queue);
		return;
	}

	cur_tcb = mut->pSleep_queue;

	while(cur_tcb != NULL) {
		prev_tcb = cur_tcb;
		cur_tcb = cur_tcb->sleep_queue;
	}

	prev_tcb->sleep_queue = target_tcb;
	target_tcb->sleep_queue = NULL;
}

int mutex_lock(int mutex)
{
	mutex_t *mut;
	tcb_t *cur_tcb;

//	printf("lock called by %u\n", get_cur_tcb()->native_prio);

	if((mutex < 0) || (mutex >= OS_NUM_MUTEX)) {
		printf("invalid mutex number passed to mutex_lock\n");
		return -EINVAL;
	}

	mut = &gtMutex[mutex];

	/*
	 * check if mutex create was called before mutex lock on this mutex
	 */
	if(mut->bAvailable == TRUE) {
//		printf("mutex lock called before mutex create on mutex %d\n", mutex);
		return -EINVAL;
	}

	/*
	 * check if the current task is already holding the mutex
	 */
	cur_tcb = get_cur_tcb();
//	printf("before checking holding tcb cut prio is %u\n", cur_tcb->native_prio);
	if(mut->pHolding_Tcb == cur_tcb) {
		printf("task %u is already holding mutex %d", cur_tcb->native_prio, 
		                                              mutex);
		return -EDEADLOCK;
	}
//	printf("after checking holding tcb cut prio is %u\n", cur_tcb->native_prio);
	
	/*
	 * check if this mutex is acquireable
	 */
	if(mut->bLock == TRUE) {
		// this mutex is already locked
		add_to_mutex_sleep_queue(mut, cur_tcb);
//		printf("after adding to sleep queue, mut->sleep_queue is %p\n", mut->pSleep_queue);
		dispatch_sleep();
	}

	/*
	 * this task is woken up. set this task as the current owner of the mutex
	 */
	mut->bLock = TRUE;
	mut->pHolding_Tcb = cur_tcb;
//	printf("\n in lock, updated holding tcb to %p\n", mut->pHolding_Tcb);
	return 0;
}

int mutex_unlock(int mutex)
{
	mutex_t *mut;
	tcb_t *cur_tcb, *next_tcb;

//	printf("unlock called by %u\n", get_cur_tcb()->native_prio);

	if((mutex < 0) || (mutex >= OS_NUM_MUTEX)) {
		printf("invalid mutex number passed to mutex_unlock\n");
		return -EINVAL;
	}

	mut = &gtMutex[mutex];

	/*
	 * check if mutex create was called before mutex unlock on this mutex
	 */
	if(mut->bAvailable == TRUE) {
		printf("mutex unlock called before mutex create on mutex %d\n", mutex);
		return -EINVAL;
	}

	/*
	 * check if the current task is already holding the mutex
	 */
	cur_tcb = get_cur_tcb();
	if(mut->pHolding_Tcb != cur_tcb) {
		printf("task %u is not the holder of mutex %d", cur_tcb->native_prio, 
		                                              mutex);
		return -EPERM;
	}

	/*
	 * mark this mutex as unlocked
	 */
	mut->bLock = FALSE;
	mut->pHolding_Tcb = NULL;
	/*
	 * wake up the first task waiting on this mutex's sleep queue
	 */
//	printf("before checking mut->sleep_queue in unlock head is %p\n", 
//	mut->pSleep_queue);
	if(mut->pSleep_queue != NULL) {
		next_tcb = mut->pSleep_queue;
		mut->pSleep_queue = next_tcb->sleep_queue;
		next_tcb->sleep_queue = NULL;
//		mut->pHodling_Tcb = next_tcb;
//		printf("in unlock addin %u to run queue\n", next_tcb->cur_prio);
		runqueue_add(next_tcb, next_tcb->cur_prio);
	}
	return 0;
}
