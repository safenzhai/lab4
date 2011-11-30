/** @file dagger.c
 *
 * @brief Creates two simple periodic tasks.
 *
 * @note  This is like knife -- but smaller :)
 *
 * @author Kartik Subramanian <ksubrama@andrew.cmu.edu>
 * @date   2008-11-30
 */
#include <stdio.h>
#include <task.h>
#include <unistd.h>


void panic(const char* str)
{
	puts(str);
	while(1);
}

void fun1(void* str)
{
	while(1)
	{
		putchar((int)str);
		putchar('\n');
		if (event_wait(0) < 0)
			panic("Dev 0 failed");
	}
}

void fun2(void* str)
{
	while(1)
	{
		putchar((int)str);
		putchar('\n');
		if (event_wait(1) < 0)
			panic("Dev 1 failed");
	}
}

void fun3(void* str)
{
	while(1)
	{
		putchar((int)str);
		putchar('\n');
		if (event_wait(2) < 0)
			panic("Dev 2 failed");
	}
}

void fun4(void* str)
{
	while(1)
	{
		putchar((int)str);
		putchar('\n');
		if (event_wait(3) < 0)
			panic("Dev 3 failed");
	}
}
int main(int argc, char** argv)
{
	task_t tasks[4];
	tasks[0].lambda = fun1;
	tasks[0].data = (void*)'@';
	tasks[0].stack_pos = (void*)0xa2000000;
	tasks[0].C = 1;
	tasks[0].T = PERIOD_DEV0;
	tasks[1].lambda = fun2;
	tasks[1].data = (void*)'#';
	tasks[1].stack_pos = (void*)0xa1000000;
	tasks[1].C = 1;
	tasks[1].T = PERIOD_DEV1;
	tasks[2].lambda = fun3;
	tasks[2].data = (void*)'*';
	tasks[2].stack_pos = (void*)0xa1800000;
	tasks[2].C = 1;
	tasks[2].T = PERIOD_DEV2;
	tasks[3].lambda = fun4;
	tasks[3].data = (void*)'%';
	tasks[3].stack_pos = (void*)0xa1c00000;
	tasks[3].C = 1;
	tasks[3].T = PERIOD_DEV3;

	task_create(tasks, 4);
	argc=argc; /* remove compiler warning */
	argv[0]=argv[0]; /* remove compiler warning */

	puts("Why did your code get here!\n");
	return 0;
}
