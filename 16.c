#include<stdio.h>
#include "userthread.h"
/*
  Expected:: I am func4\nI am func5\nI am func6\nI am func1\nI am func2\nI am func3\nMain exiting\n
*/
int ret1, ret2, ret3;
void busy_wait(int n) {
  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < 100000; j++) {
    }
  }
}

void func1(void *arg) {
  busy_wait(3);
  thread_join(ret1);
  printf("I am func1\n");
}

void func2(void *arg) {
  busy_wait(2);
  thread_join(ret2);
  printf("I am func2\n");
}

void func3(void *arg) {
  busy_wait(1);
  thread_join(ret3);
  printf("I am func3\n");
}

void func4(void *arg)  {	
  printf("I am func4\n");
}

void func5(void *arg) {
  printf("I am func5\n");
}

void func6(void *arg) {
  printf("I am func6\n");
}

int main() {
  thread_libinit(SJF);
  int retfirst = thread_create(func1, NULL, 0);
  int ret = thread_create(func2, NULL, 0);
  ret = thread_create(func3, NULL, 0);
  ret1 = thread_create(func4, NULL, 0);
  ret2 = thread_create(func5, NULL, 0);
  ret3 = thread_create(func6, NULL, 0);
  busy_wait(4);
  ret = thread_join(retfirst);
  printf("Main exiting\n");
  return 0;
}
