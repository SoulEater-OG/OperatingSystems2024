#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

void f1(void *arg) {
  printf("Finished running thread with priority -1\n");
}

void f2(void *arg) {
  printf("Finished running thread with priority 0\n");
}

void f3(void *arg) {
  printf("Finished running thread with priority 1\n", arg);
}

int main(void) {
  if (thread_libinit(PRIORITY) == -1)
    exit(EXIT_FAILURE);

  int tid1 = thread_create(f1, NULL, -1);
  int tid2 = thread_create(f2, NULL, 0);
  int tid3 = thread_create(f3, NULL, 1);

  int n = 3;
  int tids[] = { tid1, tid2, tid3};

  for (int i = 0; i < n; i++)  {
    if (tids[i] == -1)
      exit(EXIT_FAILURE);
  }

  for (int i = 0; i < n; i++)  {
    if (thread_join(tids[i]) == -1)
      exit(EXIT_FAILURE);
  }

  if (thread_libterminate() == -1)
    exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}
