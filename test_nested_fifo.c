#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

#define N 2
#define FAIL -1

/* 
  Expected:: I am in the first layer! 0\nI am in the second layer! 0\nI am in the third layer!\nI am in the second layer! 1\nI am in the third layer!\nI am in the first layer! 1\nI am in the second layer! 0\nI am in the third layer!\nI am in the second layer! 1\nI am in the third layer!\n
*/

void thirdlayer(void *arg)
{
  printf("I am in the third layer!\n");
  thread_yield();
}

void secondlayer(void *arg)
{
  int tid;
  for (int i = 0; i < N; i++)
  {
    tid = thread_create(thirdlayer, NULL, 1);
    if (tid == FAIL)
      exit(EXIT_FAILURE);
    printf("I am in the second layer! %d\n", i);
    if (thread_join(tid) == FAIL)
      exit(EXIT_FAILURE);
  }
}

void firstlayer(void *arg)
{
  int tid;
  for (int i = 0; i < N; i++)
  {
    tid = thread_create(secondlayer, NULL, 1);
    if (tid == FAIL)
      exit(EXIT_FAILURE);
    printf("I am in the first layer! %d\n", i);
    if (thread_join(tid) == FAIL)
      exit(EXIT_FAILURE);
  }
}

int main(void)
{
  if (thread_libinit(FIFO) == FAIL)
    exit(EXIT_FAILURE);

  int t = thread_create(firstlayer, NULL, 1);
  if (thread_join(t) == FAIL)
    exit(EXIT_FAILURE);

  if (thread_libterminate() == FAIL)
    exit(EXIT_FAILURE);

  exit(EXIT_SUCCESS);
}
