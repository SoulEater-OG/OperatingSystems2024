#include <stdio.h>
#include <stdlib.h>
#include <poll.h>

#include "userthread.h"

#define THREAD_NUM 5
#define SLEEP_MS 100
#define FAILURE -1

/*
  Expected:: I'm thread 1 with polling 100\nI'm thread 3 with polling 300\nI'm thread 5 with polling 500\nI'm thread 2 with polling 200\nI'm thread 4 with polling 400\n
*/

void foo(void *arg)
{
    int num = *((int *)arg);
    int polling = SLEEP_MS * num;
    poll(NULL, 0, polling);
    if (num % 2 == 0)
    {
        thread_yield();
    }
    printf("I'm thread %d with polling %d\n", num, polling);
}

int main(int argc, char **argv)
{
    if (thread_libinit(SJF) == FAILURE)
    {
        printf("init failure\n");
        exit(EXIT_FAILURE);
    }

    // thread #
    int num[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        num[i] = i + 1;
    }

    // create threads
    int threads[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; ++i)
    {
        threads[i] = thread_create(foo, num + i, 0);
        if (threads[i] == FAILURE)
        {
            printf("create failure\n");
            exit(EXIT_FAILURE);
        }
    }

    // join threads
    for (int i = THREAD_NUM - 1; i >= 0; --i)
    {
        if (thread_join(threads[i]) == FAILURE)
        {
            printf("join failure\n");
            exit(EXIT_FAILURE);
        }
    }

    if (thread_libterminate() == FAILURE)
    {
        printf("term failure\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}