#ifndef SHELL_H
#define SHELL_H

#include "vfs.h"

extern struct termios shell_tmodes;

void run_shell(int disk_fd);
void init_shell();
void handle_sigchld(int sig);
void handle_sigtstp(int sig);

#endif // SHELL_H
