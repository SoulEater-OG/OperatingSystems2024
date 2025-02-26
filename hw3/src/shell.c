#include <termios.h>  // Add this include for termios functions
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

void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        setpgid(0, 0); // New process group for the child process
        if (!background) {
            tcsetpgrp(STDIN_FILENO, getpid()); // Give child control of terminal
        }

        // Restore terminal modes
        if (!background) {
            tcsetattr(STDIN_FILENO, TCSADRAIN, &shell_tmodes);
        }

        // Handle redirection
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            } else if (strcmp(args[i], "<") == 0) {
                int fd = open(args[i + 1], O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
                args[i] = NULL;
                break;
            }
        }

        if (background) {
            // Redirect standard file descriptors to /dev/null
            int fd = open("/dev/null", O_RDWR);
            if (fd == -1) {
                perror("open /dev/null failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        if (DEBUG) printf("[DEBUG] Executing command: %s\n", args[0]);
        if (execvp(args[0], args) < 0) {
            perror("exec failed");
            exit(1);
        }
    } else {
        // Parent process
        setpgid(pid, pid); // Set child to its own process group
        if (background) {
            add_job(pid, args[0]);
            if (DEBUG) printf("[DEBUG] Started background job with PID %d\n", pid);
        } else {
            tcsetpgrp(STDIN_FILENO, pid); // Give child control of terminal
            waitpid(pid, NULL, WUNTRACED); // Wait for child to finish or stop
            tcsetpgrp(STDIN_FILENO, getpid()); // Return terminal control to shell
            // Save terminal modes
            tcgetattr(STDIN_FILENO, &shell_tmodes);
            if (DEBUG) printf("[DEBUG] Child process finished\n");
        }
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

void execute_pipe(char **cmds, int num_cmds) {
    int i;
    int pipefds[2 * (num_cmds - 1)];

    // Create all the pipes
    for (i = 0; i < (num_cmds - 1); i++) {
        if (pipe(pipefds + i*2) < 0) {
            perror("pipe");
            exit(1);
        }
    }

    int j = 0;
    for (i = 0; i < num_cmds; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process

            // Redirect stdin if it's not the first command
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], 0) < 0) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Redirect stdout if it's not the last command
            if (i < num_cmds - 1) {
                if (dup2(pipefds[i * 2 + 1], 1) < 0) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Close all pipe file descriptors
            for (j = 0; j < 2 * (num_cmds - 1); j++) {
                close(pipefds[j]);
            }

            // Parse and execute the command
            char *args[MAX_LINE / 2 + 1];
            parse_command(cmds[i], args);
            if (execvp(args[0], args) < 0) {
                perror("execvp");
                exit(1);
            }
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        }
    }

    // Close all pipe file descriptors in the parent
    for (i = 0; i < 2 * (num_cmds - 1); i++) {
        close(pipefds[i]);
    }

    // Wait for all child processes
    for (i = 0; i < num_cmds; i++) {
        wait(NULL);
    }
}

void run_shell() {
    char command[MAX_LINE];
    char *args[MAX_LINE / 2 + 1]; // Command line arguments
    char **commands = NULL;
    int num_commands = 0;

    init_shell();

    if (DEBUG) printf("[DEBUG] Shell started, entering main loop\n");

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
                execute_command(args, background);
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
