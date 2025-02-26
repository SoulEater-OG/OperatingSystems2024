#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am func1\nI am func1\nI am func1\nI am func1\nI am func1\nI am func1\nI am func2\nMain exiting\n
 */
int ret1, ret2, ret3;
void busy_wait(int n)
{
	int i, j;
	for (i = 0; i < n; i++) {
		for (j = 0; j < 100000; j++) {
		}
	}
}

void func1(void *arg)
{
	busy_wait(1);
	thread_yield();
	int i;
	for (i = 0; i < 6; i++) {
		busy_wait(1);
		printf("I am func1\n");
		thread_yield();
	}
}

void func2(void *arg)
{
	busy_wait(5);
	thread_yield();
	printf("I am func2\n");
}

int main()
{
	thread_libinit(SJF);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
