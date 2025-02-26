#include<stdio.h>
#include "userthread.h"
/*
 Expected:: I am func1\nI am func2\nI am func3\nMain exiting\n
 */
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
  thread_libinit(FIFO);
  int ret1 = thread_create(func1, NULL, 0);
  int ret2 = thread_create(func2, NULL, 0);
  int ret3 = thread_create(func3, NULL, 0);
  int ret = thread_join(ret1);
  ret = thread_join(ret2);
  ret = thread_join(ret3);
  printf("Main exiting\n");
  return 0;
}
