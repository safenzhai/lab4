/** @file proc.c
 * 
 * @brief Implementation of `process' syscalls
 *
 * @author Mike Kasick <mkasick@andrew.cmu.edu>
 * @date   Sun, 14 Oct 2007 00:07:38 -0400
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date 2008-11-12
 */

#include <exports.h>
#include <bits/errno.h>
#include <config.h>
#include <kernel.h>
#include <syscall.h>
#include <sched.h>
#include <types.h>
#include <../sched/sched_i.h>

#include <arm/reg.h>
#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/physmem.h>
#include <device.h>

extern void print_run_queue(void);

int validate_all_tasks(task_t *tasks, size_t num_tasks)
{
	unsigned int i;
	for(i = 0; i < num_tasks ; i++) {
		// validate lambda
		if(valid_addr(tasks[i].lambda, sizeof(void *), 
		          USR_START_ADDR, USR_END_ADDR) == 0) {
			printf("lambda for task %d is invalid\n", i);
			return -1;
		}

		// validate stack_pos
		if(valid_addr(tasks[i].stack_pos, sizeof(void *), 
		          USR_START_ADDR, USR_END_ADDR) == 0) {
			printf("stack_pos for task %d is invalid\n", i);
			return -1;
		}

		//validate C and T
		if(tasks[i].C == 0) {
			printf("C for task %d is invalid\n", i);
			return -1;
		}
		if(tasks[i].T == 0) {
			printf("T for task %d is invalid\n", i);
			return -1;
		}
		if(tasks[i].C > tasks[i].T) {
			printf("C > T for task %d is invalid\n", i);
			return -1;
		}
	}
	// all set!
	return 0;
}

void swap_tasks(task_t *tasks, int i, int j)
{
	task_t temp;
	// do a deep copy of the entire struct
	temp.lambda = tasks[i].lambda;
	temp.data = tasks[i].data;
	temp.stack_pos = tasks[i].stack_pos;
	temp.C = tasks[i].C;
	temp.T = tasks[i].T;

	tasks[i].lambda = tasks[j].lambda;
	tasks[i].data = tasks[j].data;
	tasks[i].stack_pos = tasks[j].stack_pos;
	tasks[i].C = tasks[j].C;
	tasks[i].T = tasks[j].T;

	tasks[j].lambda = temp.lambda;
	tasks[j].data = temp.data;
	tasks[j].stack_pos = temp.stack_pos;
	tasks[j].C = temp.C;
	tasks[j].T = temp.T;
}

int sort_all_tasks(task_t *tasks, size_t num_tasks)
{
	unsigned int i, j;
	for(i = 0; i < num_tasks; i++) {
		for(j = i + 1; j < num_tasks; j++) {
			/*
			 * check for duplidate stack_pos we only check for stack bottom
			 * we are assuming that the stacks dont overlap
			 */
			if(tasks[i].stack_pos == tasks[j].stack_pos) {
				printf("tasks %d and %d have the same stack_pos %p\n", 
				        i, j, tasks[i].stack_pos);
				return -1;
			}
			if(tasks[i].T > tasks[j].T) {
				swap_tasks(tasks, i, j);
			}
		}
	}
	// all set!
	return 0;
}

int task_create(task_t* tasks, size_t num_tasks)
{
	int ret;
	unsigned int i = 0;
	task_t idle_task;
	disable_interrupts();
	/*
	 * validate the tasks pointer and num_tasks
	 */
	if((num_tasks == 0) || (num_tasks > (OS_MAX_TASKS - 2))) {
		return -EINVAL;
	}
	ret = valid_addr(tasks, (num_tasks * sizeof(task_t)), USR_START_ADDR,
	                 USR_END_ADDR);
	if(ret == 0) {
		printf("incorrect address of tasks passed to task create\n");
		return -EFAULT;
	}

	/*
	 * validate the contents of all task_t entries
	 */
	ret = validate_all_tasks(tasks, num_tasks);
	if(ret < 0) {
		printf("invalid entries mentioned in one of the tasks\n");
		return -EINVAL;
	}

	/*
	 * sort all tasks according to their periods
	 */
	ret = sort_all_tasks(tasks, num_tasks);
	if(ret < 0) {
		return -EINVAL;
	}
	
	/*
	 * setup the run queues
	 */
	runqueue_init();

	/*
	 * allocate the tcb's for all tasks
	 */
	allocate_tasks(&tasks, num_tasks);
	
	/*
	 * initialize the idle task
	 */
	sched_init(&idle_task);

	for(i = 1; i <= num_tasks; i++) {
		runqueue_add(&system_tcb[i], system_tcb[i].native_prio);
	}
	
	print_run_queue();
	/*
	 * dispatch no save to launch the highest prio task 
	 */
	dispatch_init(&system_tcb[IDLE_PRIO]);
	dispatch_nosave();
    return 1; /* remove this line after adding your code */
}

int event_wait(unsigned int dev)
{
  return 1; /* remove this line after adding your code */	
}

/* An invalid syscall causes the kernel to exit. */
void invalid_syscall(unsigned int call_num)
{
	printf("Kernel panic: invalid syscall -- 0x%08x\n", call_num);

	disable_interrupts();
	while(1);
}
