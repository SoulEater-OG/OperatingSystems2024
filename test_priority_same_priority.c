#include <stdio.h>
#include <stdlib.h>
#include "userthread.h"

void func1(void *arg) {
  printf("I am func1\n");
}

void func2(void *arg) {
  printf("I am func2\n");
}

void func3(void *arg) {
  printf("I am func3\n");
}

int main() {
  if (thread_libinit(PRIORITY) == -1) exit(EXIT_FAILURE);

  int ret = thread_create(func1, NULL, 0);
  ret = thread_join(ret);
  ret = thread_create(func2, NULL, 0);
  ret = thread_join(ret);
  ret = thread_create(func3, NULL, 0);
  ret = thread_join(ret);

  printf("Main exiting\n");

  return 0;
}
