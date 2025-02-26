#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am func\nMain exiting - Join returned : -1\n
 */
void func(void *arg)
{
	printf("I am func\n");
}

int main()
{
	thread_libinit(FIFO);
	int ret = thread_create(func, NULL, 0);
	int j_ret = thread_join(ret+1);
	printf("Main exiting - Join returned : %d\n", j_ret);
	return 0;
}
