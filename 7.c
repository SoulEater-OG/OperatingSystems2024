#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am thread 0\nI am thread 1\nI am thread 2\nI am thread 3\nI am thread 4\nI am thread 5\nI am thread 6\nI am thread 7\nI am thread 8\nI am thread 9\nI am thread 10\nI am thread 11\nI am thread 12\nI am thread 13\nI am thread 14\nI am thread 15\nI am thread 16\nI am thread 17\nI am thread 18\nI am thread 19\nI am thread 20\nI am thread 21\nI am thread 22\nI am thread 23\nI am thread 24\nI am thread 25\nI am thread 26\nI am thread 27\nI am thread 28\nI am thread 29\nI am thread 30\nI am thread 31\nI am thread 32\nI am thread 33\nI am thread 34\nI am thread 35\nI am thread 36\nI am thread 37\nI am thread 38\nI am thread 39\nI am thread 40\nI am thread 41\nI am thread 42\nI am thread 43\nI am thread 44\nI am thread 45\nI am thread 46\nI am thread 47\nI am thread 48\nI am thread 49\nI am thread 50\nI am thread 51\nI am thread 52\nI am thread 53\nI am thread 54\nI am thread 55\nI am thread 56\nI am thread 57\nI am thread 58\nI am thread 59\nI am thread 60\nI am thread 61\nI am thread 62\nI am thread 63\nI am thread 64\nI am thread 65\nI am thread 66\nI am thread 67\nI am thread 68\nI am thread 69\nI am thread 70\nI am thread 71\nI am thread 72\nI am thread 73\nI am thread 74\nI am thread 75\nI am thread 76\nI am thread 77\nI am thread 78\nI am thread 79\nI am thread 80\nI am thread 81\nI am thread 82\nI am thread 83\nI am thread 84\nI am thread 85\nI am thread 86\nI am thread 87\nI am thread 88\nI am thread 89\nI am thread 90\nI am thread 91\nI am thread 92\nI am thread 93\nI am thread 94\nI am thread 95\nI am thread 96\nI am thread 97\nI am thread 98\nMain exiting\n
 */
static int count = 0;
void func(void *arg)
{
	printf("I am thread %d\n", count++);
}

int main()
{
	thread_libinit(FIFO);
	int ret, j_ret, i;
	for (i = 0 ;i < 99; i++) {
		ret = thread_create(func, NULL, 0);
	}
	j_ret = thread_join(ret);
	printf("Main exiting\n");
	return 0;
}
