/**
 * @file device.c
 *
 * @brief Implements simulated devices.
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-12-01
 */

#include <types.h>
#include <assert.h>

#include <task.h>
#include <sched.h>
#include <device.h>
#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>

/**
 * @brief Fake device maintainence structure.
 * Since our tasks are periodic, we can represent 
 * tasks with logical devices. 
 * These logical devices should be signalled periodically 
 * so that you can instantiate a new job every time period.
 * Devices are signaled by calling dev_update 
 * on every timer interrupt. In dev_update check if it is 
 * time to create a tasks new job. If so, make the task runnable.
 * There is a wait queue for every device which contains the tcbs of
 * all tasks waiting on the device event to occur.
 */

struct dev
{
	tcb_t* sleep_queue;
	unsigned long   next_match;
};
typedef struct dev dev_t;

/* devices will be periodically signaled at the following frequencies */
const unsigned long dev_freq[NUM_DEVICES] = {100, 200, 500, 50};
static dev_t devices[NUM_DEVICES];

/**
 * @brief Initialize the sleep queues and match values for all devices.
 */
void dev_init(void)
{
	int i;
	for(i = 0; i < NUM_DEVICES; i++) {
		devices[i].next_match = dev_freq[i];
		devices[i].sleep_queue = NULL;
	}
}


/**
 * @brief Puts a task to sleep on the sleep queue until the next
 * event is signalled for the device.
 *
 * @param dev  Device number.
 */
void dev_wait(unsigned int dev)
{
//	printf("dev wait called for dev %u by task %u\n", dev, get_cur_tcb()->cur_prio);
	tcb_t *cur_tcb = get_cur_tcb();

	/*
	 * Add the current task to the head of the sleep queue of device dev
	 */
	cur_tcb->sleep_queue = devices[dev].sleep_queue;
	devices[dev].sleep_queue = cur_tcb;
    /*
	 * put this task to sleep and run the next highest priority task
	 */
	dispatch_sleep();
}


/**
 * @brief Signals the occurrence of an event on all applicable devices. 
 * This function should be called on timer interrupts to determine that 
 * the interrupt corresponds to the event frequency of a device. If the 
 * interrupt corresponded to the interrupt frequency of a device, this 
 * function should ensure that the task is made ready to run 
 */
void dev_update(unsigned long millis)
{
	int i;
	tcb_t *temp_tcb;
//	printf("dev update called with millis %lu\n dev[0].next_match is %lu", millis, devices[0].next_match);
	/*
	 * for each device, check if its next match value matches the current
	 * time in millis. If so, wake up all tasks sleeping on that device.
	 * update next match for that device. 
	 */

	for(i = 0; i < NUM_DEVICES; i++) {
	 	if(devices[i].next_match == millis) {
//			printf("\n next_match match for device %d\n", i);
			while(devices[i].sleep_queue != NULL) {
				temp_tcb = devices[i].sleep_queue;
//				printf("\n adding task %u to run_queue\n", temp_tcb->cur_prio);
				runqueue_add(temp_tcb, temp_tcb->cur_prio);
				devices[i].sleep_queue = temp_tcb->sleep_queue;
				temp_tcb->sleep_queue = NULL;
			}

			/*
			 * check for integer overflow with next_match
			 */
			if((devices[i].next_match + dev_freq[i]) < devices[i].next_match) {
				printf("OVERFLOW IN NEXT_MATCH OF DEVICE %d \n", i);
				printf("THIS DEVICE IS NOT USABLE ANYMORE. \n");
			}
			devices[i].next_match += dev_freq[i];
		}
	}
}

