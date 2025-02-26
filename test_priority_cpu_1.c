#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define NUM_POLL 200
#define POLL_TIMEOUT 200

/* Expected %CPU: 79%, two busy-waiting threads (-1 and 0) and one sleeping thread (1)*/
void f1(void *arg) {
  while (1) {}
}

void f2(void *arg) {
  for (int i = 0; i < NUM_POLL; i++) {
    poll(NULL, 0, POLL_TIMEOUT);
  }
}

int main(void) {
  if (thread_libinit(PRIORITY) == -1)
    exit(EXIT_FAILURE);

  int tid1 = thread_create(f1, NULL, -1);
  int tid2 = thread_create(f1, NULL, 0);
  int tid3 = thread_create(f2, NULL, 1);

  int n = 3;
  int tids[] = { tid1, tid2, tid3 };

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
