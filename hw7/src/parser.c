#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>       // For file control options
#include <termios.h>     // For terminal control
#include "parser.h"
#include "job_control.h"
#include "vfs.h"

#define MAX_LINE 1024 // Maximum length of a command
#define DEBUG 1 // Enable debug printouts

extern struct termios shell_tmodes;  // Declare the external variable

void split_commands(char *input, char ***commands, int *num_commands) {
    *num_commands = 0;
    *commands = NULL;
    char *cmd = strtok(input, ";");
    while (cmd != NULL) {
        *commands = realloc(*commands, sizeof(char*) * (*num_commands + 1));
        (*commands)[*num_commands] = strdup(cmd);
        (*num_commands)++;
        cmd = strtok(NULL, ";");
    }
}

void parse_command(char *command, char **args) {
    int i = 0;
    char *token = strtok(command, " \t\n");
    while (token != NULL) {
        if (token[0] == '\"') {
            char *end_quote = strchr(token + 1, '\"');
            if (end_quote) {
                *end_quote = '\0';
                args[i++] = token + 1;
            } else {
                args[i++] = token + 1;
                token = strtok(NULL, " \t\n");
                while (token && !strchr(token, '\"')) {
                    strcat(args[i - 1], " ");
                    strcat(args[i - 1], token);
                    token = strtok(NULL, " \t\n");
                }
                if (token) {
                    end_quote = strchr(token, '\"');
                    if (end_quote) {
                        *end_quote = '\0';
                        strcat(args[i - 1], " ");
                        strcat(args[i - 1], token);
                    }
                }
            }
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

void bg_command(char **args) {
    job_t *job = job_list;
    if (args[1] != NULL) {
        int job_number = atoi(args[1] + 1); // Assuming %<job_number> format
        for (int i = 1; i < job_number && job != NULL; i++) {
            job = job->next;
        }
    }
    if (job) {
        kill(job->pid, SIGCONT);
        job->stopped = 0;
        printf("[%d] %s\n", 1, job->command); // Displaying first job for now
    } else {
        fprintf(stderr, "bg: job not found\n");
    }
}

void fg_command(char **args) {
    job_t *job = job_list;
    if (args[1] != NULL) {
        int job_number = atoi(args[1] + 1); // Assuming %<job_number> format
        for (int i = 1; i < job_number && job != NULL; i++) {
            job = job->next;
        }
    }
    if (job) {
        kill(job->pid, SIGCONT);
        job->stopped = 0;
        tcsetpgrp(STDIN_FILENO, job->pid); // Give job control of terminal
        waitpid(job->pid, NULL, WUNTRACED); // Wait for job to finish or stop
        tcsetpgrp(STDIN_FILENO, getpid()); // Return terminal control to shell
    } else {
        fprintf(stderr, "fg: job not found\n");
    }
}

void jobs_command() {
    print_jobs();
}

void kill_command(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: kill %%<job_number> [-9]\n");
        return;
    }

    int sig = SIGTERM;
    if (args[2] != NULL && strcmp(args[2], "-9") == 0) {
        sig = SIGKILL;
    }

    job_t *job = job_list;
    int job_number = atoi(args[1] + 1); // Assuming %<job_number> format
    for (int i = 1; i < job_number && job != NULL; i++) {
        job = job->next;
    }

    if (job) {
        kill(job->pid, sig);
        if (sig == SIGKILL) {
            remove_job(job->pid);
        }
    } else {
        fprintf(stderr, "kill: job not found\n");
    }
}

void split_pipes(char *command, char ***cmds, int *num_cmds) {
    *num_cmds = 0;
    *cmds = NULL;
    char *cmd = strtok(command, "|");
    while (cmd != NULL) {
        *cmds = realloc(*cmds, sizeof(char*) * (*num_cmds + 1));
        (*cmds)[*num_cmds] = strdup(cmd);
        (*num_cmds)++;
        cmd = strtok(NULL, "|");
    }
}

void execute_command(char **args, int background, vnode_t *vn) {
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

        // Handle redirection
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                int fd = vn->driver->f_open(vn, args[i + 1], O_WRONLY | O_CREAT | O_TRUNC);
                if (fd == -1) {
                    perror("vfs_open");
                    exit(1);
                }
                vn->driver->f_write(vn, args[i + 1], STDIN_FILENO, 1, args[i + 1]); // Write from stdin to the file
                vn->driver->f_close(vn, fd);
                args[i] = NULL;
                break;
            } else if (strcmp(args[i], "<") == 0) {
                int fd = vn->driver->f_open(vn, args[i + 1], O_RDONLY);
                if (fd == -1) {
                    perror("vfs_open");
                    exit(1);
                }
                vn->driver->f_read(vn, args[i + 1], STDOUT_FILENO, 1); // Read from the file to stdout
                vn->driver->f_close(vn, fd);
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
