#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am func3\nI am func2\nI am func1\nMain exiting\n
 */
int join[2];
void func1(void *arg)
{
	int ret = thread_join(join[0]);
	printf("I am func1\n");
}

void func2(void *arg)
{
	int ret = thread_join(join[1]);
	printf("I am func2\n");
}

void func3(void *arg)
{
	printf("I am func3\n");
}

int main()
{
	thread_libinit(FIFO);
	int ret1 = thread_create(func1, NULL, 0);
	int ret = thread_create(func2, NULL, 0);
	join[0] = ret;
	ret = thread_create(func3, NULL, 0);
	join[1] = ret;
	ret = thread_join(ret1);
	printf("Main exiting\n");
	return 0;
}
