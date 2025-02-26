#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am func1\nI am func2\nI am func3\nMain exiting\n
 */
void func1(void *arg)
{
	thread_yield();
	printf("I am func1\n");
}

void func2(void *arg)
{
	thread_yield();
	printf("I am func2\n");
}

void func3(void *arg)
{
	thread_yield();
	printf("I am func3\n");
}

int main()
{
	thread_libinit(FIFO);
	int ret = thread_create(func1, NULL, 0);
	ret = thread_create(func2, NULL, 0);
	ret = thread_create(func3, NULL, 0);
	ret = thread_join(ret);
	printf("Main exiting\n");
	return 0;
}
