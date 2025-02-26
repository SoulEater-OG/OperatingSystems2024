#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define FAILURE -1

void f(void* arg) {
  thread_libterminate();
}

int main(int args, char** argv) {
  if(thread_libinit(FIFO) == -1)
    exit(EXIT_FAILURE);

  int tid1 = thread_create(f,NULL,0);
  int tid2 = thread_create(f,NULL,0);
  if (tid1 == FAILURE || tid2 == FAILURE) exit(EXIT_FAILURE);

  thread_join(tid1);
  thread_join(tid2);

  if(thread_libterminate() == -1) {
    printf("libterminate failed\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
