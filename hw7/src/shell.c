#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "shell.h"
#include "parser.h"
#include "job_control.h"
#include "vfs.h"
#include "commands.h"

#define MAX_LINE 1024 // Maximum length of a command
#define DEBUG 1 // Set to 1 to enable debug printouts, 0 to disable

// Global variables for terminal management
int shell_terminal;
pid_t shell_pgid;
struct termios shell_tmodes;

// Signal handlers
void handle_sigchld(int sig) {
    (void)sig; // Mark the parameter as used
    if (DEBUG) printf("[DEBUG] Received SIGCHLD signal\n");

    sigset_t mask, prev_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prev_mask); // Block SIGCHLD

    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            if (DEBUG) printf("[DEBUG] Child process %d terminated\n", pid);
            remove_job(pid);
        } else if (WIFSTOPPED(status)) {
            if (DEBUG) printf("[DEBUG] Child process %d stopped\n", pid);
            update_job_status(pid, 1);
        }
    }

    sigprocmask(SIG_SETMASK, &prev_mask, NULL); // Restore previous signal mask
}

void handle_sigtstp(int sig) {
    (void)sig; // Mark the parameter as used
    if (DEBUG) printf("[DEBUG] Received SIGTSTP signal\n");
    pid_t pid = tcgetpgrp(STDIN_FILENO);
    if (pid != getpid()) {
        kill(-pid, SIGTSTP);
        update_job_status(pid, 1);
    }
}

void init_shell() {
    // Ensure the shell is running in its own process group
    shell_terminal = STDIN_FILENO;
    shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("Failed to put the shell in its own process group");
        exit(1);
    }
    tcsetpgrp(shell_terminal, shell_pgid);

    // Save terminal modes
    tcgetattr(shell_terminal, &shell_tmodes);

    // Ignore job control signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGCHLD, handle_sigchld);
}

void run_shell(int disk_fd) {
    char command[MAX_LINE];
    char *args[MAX_LINE / 2 + 1]; // Command line arguments
    char **commands = NULL;
    int num_commands = 0;

    init_shell();

    if (DEBUG) printf("[DEBUG] Shell started, entering main loop\n");

    vnode_t vn;
    vn.fd = disk_fd;
    vn.driver = &fat16_driver; // Assign the FAT-16 driver to the vnode

    while (1) {
        printf("mysh> ");
        if (fgets(command, MAX_LINE, stdin) == NULL) {
            if (DEBUG) printf("[DEBUG] EOF detected, exiting shell\n");
            break; // Exit on EOF
        }

        // Remove trailing newline character
        command[strcspn(command, "\n")] = '\0';

        if (DEBUG) printf("[DEBUG] Full input command: %s\n", command);

        // Split commands by ';'
        split_commands(command, &commands, &num_commands);

        for (int j = 0; j < num_commands; j++) {
            char *cmd = commands[j];

            if (DEBUG) printf("[DEBUG] Processing command: %s\n", cmd);

            // Remove leading and trailing whitespace from each command
            while (isspace((unsigned char)*cmd)) cmd++;
            char *end = cmd + strlen(cmd) - 1;
            while (end > cmd && isspace((unsigned char)*end)) end--;
            *(end + 1) = '\0';

            if (DEBUG) printf("[DEBUG] Trimmed command: %s\n", cmd);

            // Check for pipes
            char **pipe_cmds = NULL;
            int num_pipe_cmds = 0;
            split_pipes(cmd, &pipe_cmds, &num_pipe_cmds);

            if (num_pipe_cmds > 1) {
                // Execute piped commands
                execute_pipe(pipe_cmds, num_pipe_cmds);

                // Free allocated memory for pipe commands
                for (int k = 0; k < num_pipe_cmds; k++) {
                    free(pipe_cmds[k]);
                }
                free(pipe_cmds);
            } else {
                // No pipe, execute as usual
                parse_command(cmd, args);

                // Print args array for debugging
                if (DEBUG) {
                    printf("[DEBUG] Parsed arguments:\n");
                    for (int i = 0; args[i] != NULL; i++) {
                        printf("[DEBUG] args[%d] = %s\n", i, args[i]);
                    }
                }

                if (args[0] == NULL) {
                    if (DEBUG) printf("[DEBUG] Empty command, continuing\n");
                    continue; // Empty command
                }

                if (strcmp(args[0], "exit") == 0) {
                    if (DEBUG) printf("[DEBUG] Exit command detected, exiting shell\n");
                    // Free allocated memory for commands
                    for (int k = 0; k < num_commands; k++) {
                        free(commands[k]);
                    }
                    free(commands);
                    return; // Exit the shell
                }

                if (strcmp(args[0], "bg") == 0) {
                    if (DEBUG) printf("[DEBUG] bg command detected\n");
                    bg_command(args);
                    continue;
                }

                if (strcmp(args[0], "fg") == 0) {
                    if (DEBUG) printf("[DEBUG] fg command detected\n");
                    fg_command(args);
                    continue;
                }

                if (strcmp(args[0], "jobs") == 0) {
                    if (DEBUG) printf("[DEBUG] jobs command detected\n");
                    jobs_command();
                    continue;
                }

                if (strcmp(args[0], "kill") == 0) {
                    if (DEBUG) printf("[DEBUG] kill command detected\n");
                    kill_command(args);
                    continue;
                }

                if (strcmp(args[0], "ls") == 0) {
                    if (DEBUG) printf("[DEBUG] ls command detected\n");
                    ls_command(&vn, args);
                    continue;
                }

                if (strcmp(args[0], "cat") == 0) {
                    if (DEBUG) printf("[DEBUG] cat command detected\n");
                    cat_command(&vn, args);
                    continue;
                }

                if (strcmp(args[0], "write") == 0) {
                    if (DEBUG) printf("[DEBUG] write command detected\n");
                    write_command(&vn, args);
                    continue;
                }

                if (strcmp(args[0], "read") == 0) {
                    if (DEBUG) printf("[DEBUG] read command detected\n");
                    read_command(&vn, args);
                    continue;
                }

                int background = 0;
                int i = 0;
                while (args[i] != NULL) {
                    if (strcmp(args[i], "&") == 0) {
                        background = 1;
                        args[i] = NULL;
                        break;
                    }
                    i++;
                }

                if (DEBUG) printf("[DEBUG] Running command: %s\n", cmd);
                execute_command(args, background, &vn);
            }
        }

        // Free allocated memory for commands
        for (int k = 0; k < num_commands; k++) {
            free(commands[k]);
        }
        free(commands);
        commands = NULL;
        num_commands = 0;
    }

    if (DEBUG) printf("[DEBUG] Shell exited\n");
}