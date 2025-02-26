#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define NUM_POLL 10
#define POLL_TIMEOUT 200

/* should be scheduled 11 times */
void f(void *arg) {
  for (int i = 0; i < NUM_POLL; i++) {
    poll(NULL, 0, POLL_TIMEOUT);
  }
  printf("Finished running thread with priority -1\n");
}

int main(void) {
  if (thread_libinit(PRIORITY) == -1)
    exit(EXIT_FAILURE);

  int tid = thread_create(f, NULL, -1);

  if (tid == -1)
    exit(EXIT_FAILURE);

  if (thread_join(tid) == -1)
    exit(EXIT_FAILURE);

  if (thread_libterminate() == -1)
    exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}
