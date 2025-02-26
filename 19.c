#include<stdio.h>
#include "userthread.h"
/*
 Expected:: func1 before calling yield\nI am func1\nI am func2\nMain exiting\n
 */
int ret;
void func2(void *arg)
{
	int ret1 = thread_join(ret);
	printf("I am func2\n");
}

void func1(void *arg)
{
	printf("func1 before calling yield\n");
	thread_yield();
	printf("I am func1\n");
}

int main()
{
	thread_libinit(FIFO);
	ret = thread_create(func1, NULL, 0);
	int ret1 = thread_create(func2, NULL, 0);
	ret1 = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
