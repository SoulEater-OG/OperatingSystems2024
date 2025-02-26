#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

void f(void *arg) {
  printf("This should not be printed\n");
}

int main(void) {
  if (thread_libinit(PRIORITY) == -1)
    exit(EXIT_FAILURE);

  int tid = thread_create(f, NULL, -2);

  if (tid == -1)
    exit(EXIT_FAILURE);

  if (thread_join(tid) == -1)
    exit(EXIT_FAILURE);

  if (thread_libterminate() == -1)
    exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}
