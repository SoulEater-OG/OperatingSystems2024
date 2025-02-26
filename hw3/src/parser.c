#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "parser.h"
#include "job_control.h"

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
    args[i] = strtok(command, " \t\n");
    while (args[i] != NULL) {
        args[++i] = strtok(NULL, " \t\n");
    }
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
