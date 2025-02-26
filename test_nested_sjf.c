#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

/*
  Expected:: I am in the first layer haha! 0\nI am in the second layer haha! 0\nI am in the third layer haha! 0\nI am in the first layer haha! 1\nI am in the second layer haha! 1\nI am in the third layer haha! 1\n
*/

void third_layer(void *arg)
{
    printf("I am in the third layer haha! %d\n", *(int*) arg);
    thread_yield();
}

void second_layer(void *arg)
{
    int tid = thread_create(third_layer, arg, 1);
    if (tid < 0)
    {
        exit(EXIT_FAILURE);
    }
    printf("I am in the second layer haha! %d\n", *(int*) arg);
    if (thread_join(tid) < 0)
    {
        exit(EXIT_FAILURE);
    }
}

void first_layer(void *arg)
{
    for (int i = 0; i < 2; ++i)
    {
        int tid = thread_create(second_layer, &i, 1);
        if (tid < 0)
        {
            exit(EXIT_FAILURE);
        }
        printf("I am in the first layer haha! %d\n", i);
        if (thread_join(tid) < 0)
        {
            exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    if (thread_libinit(SJF) == -1)
        exit(EXIT_FAILURE);

    int tid1 = thread_create(first_layer, NULL, 1);

    if (thread_join(tid1) < 0)
    {
        exit(EXIT_FAILURE);
    }

    if (thread_libterminate() == -1)
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}