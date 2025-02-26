#include<stdio.h>
#include "userthread.h"
/*
 Expected:: func1 before calling yield\nI am func1\nfunc2 before calling yield\nfunc3 before calling yield\nMain exiting\n
 */
int join;
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
	printf("func1 before calling yield\n");
	busy_wait(10);
	thread_yield();
	printf("I am func1\n");
}

void func2(void *arg)
{
	busy_wait(2);
	thread_yield();
	printf("func2 before calling yield\n");
	busy_wait(5);
	thread_yield();
	printf("I am func2\n");
}

void func3(void *arg)
{
	busy_wait(3);
	thread_yield();
	printf("func3 before calling yield\n");
	busy_wait(1);
	thread_yield();
	printf("I am func3\n");
}

int main()
{
	thread_libinit(SJF);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	ret = thread_create(func3, NULL, 0);
	join = ret;
	busy_wait(5);
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
