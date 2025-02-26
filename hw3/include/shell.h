#ifndef SHELL_H
#define SHELL_H

#define DEBUG 1

void run_shell();
void handle_sigchld(int sig);
void handle_sigtstp(int sig);
void execute_pipe(char **cmds, int num_cmds);
void execute_command(char **args, int background);
void parse_command(char *command, char **args);

#endif // SHELL_H
