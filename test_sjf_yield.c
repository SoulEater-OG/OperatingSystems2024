#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include "userthread.h"

#define N 5
#define FAIL -1

/*
  Expected:: I am running wisefool50!\nI am running wisefool200!\nI am running wisefool100!\nI am running wisefool300!\nI am running wisefool50!\nI am running wisefool50!\nI am running wisefool50!\nI am running wisefool50!\nI am running wisefool100!\nI am running wisefool100!\nI am running wisefool100!\nI am running wisefool100!\nI am running wisefool200!\nI am running wisefool200!\nI am running wisefool200!\nI am running wisefool200!\nI am running wisefool300!\nI am running wisefool300!\nI am running wisefool300!\nI am running wisefool300!\nMain exiting\n
*/

void wisefool50(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool50!\n");
    poll(NULL, 0, 50);
    thread_yield();
  }
}

void wisefool100(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool100!\n");
    poll(NULL, 0, 100);
    thread_yield();
  }
}

void wisefool200(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool200!\n");
    poll(NULL, 0, 200);
    thread_yield();
  }
}

void wisefool300(void *args) {
  for (int i = 0; i < N; i++) {
    printf("I am running wisefool300!\n");
    poll(NULL, 0, 300);
    thread_yield();
  }
}

int main(void)
{
    if (thread_libinit(SJF) == FAIL)
        exit(EXIT_FAILURE);
    int tid1 = thread_create(wisefool50, NULL, 0);
    int tid2 = thread_create(wisefool200, NULL, 0);
    int tid3 = thread_create(wisefool100, NULL, 0);
    int tid4 = thread_create(wisefool300, NULL, 0);

    int num = 4;
    int tids[] = {tid1, tid2, tid3, tid4};
    for (int i = 0; i < num; i++)
    {
        if (tids[i] == FAIL)
            exit(EXIT_FAILURE);
    }
    for (int i = 0; i < num; i++)
    {
        if (thread_join(tids[i]) == FAIL)
            exit(EXIT_FAILURE);
    }
    if (thread_libterminate() == FAIL)
        exit(EXIT_FAILURE);
    printf("Main exiting\n");
    exit(EXIT_SUCCESS);
}
