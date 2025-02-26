#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include "userthread.h"

#define NUM_YIELD 100

/* Expected finishing order: 1 -> 0 -> -1 */
void f1(void *arg) {
  int tid = *(int*)arg;
  for (int i = 0; i < NUM_YIELD; i++) {
    thread_yield();
  }
  thread_join(tid);
}

void f2(void *arg) {}

void f3(void *arg) {
  int tid = *(int*)arg;
  for (int i = 0; i < NUM_YIELD * 2; i++) {
    thread_yield();
  }
  thread_join(tid);
}

int main(void) {
  if (thread_libinit(PRIORITY) == -1){
    exit(EXIT_FAILURE);
  }

  int tid1 = thread_create(f2, NULL, 1);
  
  int arg2 = 1, arg3 = 2;
  int tid2 = thread_create(f1, (void*)&tid1, 0);
  int tid3 = thread_create(f3, (void*)&tid2, -1);

  if (tid1 == -1 || tid2 == -1 || tid3 == -1) exit(EXIT_FAILURE);

  if (thread_join(tid1) == -1) exit(EXIT_FAILURE);
  if (thread_join(tid2) == -1) exit(EXIT_FAILURE);
  if (thread_join(tid3) == -1) exit(EXIT_FAILURE);

  if (thread_libterminate() == -1){
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
}
