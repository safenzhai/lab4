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

unsigned int *x = 0xa2fffaf0;
int mutex = -1;

void panic(const char* str)
{
	puts(str);
	while(1);
}


void delay(void)
{
	int i, j;
	for(i = 0; i < 10000; i++) {
		for(j = 0; j < 10000; j++) {
		}
	}
}

void fun1(void* str)
{
	int i;
	unsigned int y;
	puts("inside fun1\n");
//	mutex = mutex_create();
		for(i = 0; i < 10000; i++) {
			mutex_lock(mutex);
			y = *x;
			delay();
			y++;
			delay();
			*x = y;
			mutex_unlock(mutex);
		}
		puts("fun 1 calling sleep\n");
//		sleep(x);
		while(1);
}

void fun2(void* str)
{
	int i;
	unsigned int y;
//	event_wait(0);
	mutex = mutex_create();
	puts("inside fun2\n");
		for(i = 0; i < 10000; i++) {
			mutex_lock(mutex);
			y = *x;
			delay();
			y++;
			delay();
			*x = y;
			mutex_unlock(mutex);
		}
		puts("fun 2 calling sleep\n");
//		sleep(x);
		while(1);
}

int main(int argc, char** argv)
{
	puts("inside mutex app\n");
	*x = 0;
	task_t tasks[2];
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
	
	puts("gonna call task create");
	task_create(tasks, 2);
	puts("called task create");
	argc=argc; /* remove compiler warning */
	argv[0]=argv[0]; /* remove compiler warning */

	puts("Why did your code get here!\n");
	return 0;
}
