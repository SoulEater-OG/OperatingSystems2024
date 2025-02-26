/*
  THIS IS A TEST FOR THE ROBUSTNESS OF THE LIBRARY
  Expect Behavior: The program should not crash or
  cause memory leaks.
*/

#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define FAILURE -1

void f(void *arg) {}

int main(void) {
  if (thread_create(f, NULL, 0) != -1) {
    exit(EXIT_FAILURE);
  }
  if (thread_join(1) != -1) {
    exit(EXIT_FAILURE);
  }
  if (thread_yield() != -1) {
    exit(EXIT_FAILURE);
  }
  thread_libterminate();

  if (thread_libinit(FIFO) == -1) {
    exit(EXIT_FAILURE);
  }

  int tid1 = thread_create(f, NULL, 0);
  int tid2 = thread_create(f, NULL, 0);
  int tid3 = thread_create(f, NULL, 0);
  if (tid1 == FAILURE || tid2 == FAILURE || tid3 == FAILURE) exit(EXIT_FAILURE);

  if (thread_join(2018) != -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_join(tid1) == -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_join(tid2) == -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_join(tid3) == -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_libterminate() == -1)
    exit(EXIT_FAILURE);

  thread_libterminate();

  if (thread_create(f, NULL, 0) != -1) {
    exit(EXIT_FAILURE);
  }
  if (thread_join(1) != -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_libinit(SJF) == -1) {
    exit(EXIT_FAILURE);
  }

  if (thread_libinit(SJF) != -1) {
    exit(EXIT_FAILURE);
  }

  thread_libterminate();

  exit(EXIT_SUCCESS);
}
