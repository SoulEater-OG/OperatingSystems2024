#include "job_control.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

job_t *job_list = NULL; // Define job_list

void add_job(pid_t pid, const char *command) {
    job_t *new_job = (job_t *)malloc(sizeof(job_t));
    new_job->pid = pid;
    strncpy(new_job->command, command, 255);
    new_job->command[255] = '\0';
    new_job->stopped = 0;
    new_job->next = job_list;
    job_list = new_job;
}

void remove_job(pid_t pid) {
    job_t **current = &job_list;
    while (*current) {
        job_t *entry = *current;
        if (entry->pid == pid) {
            *current = entry->next;
            free(entry);
            return;
        }
        current = &entry->next;
    }
}

void print_jobs() {
    job_t *current = job_list;
    int job_number = 1;
    while (current) {
        printf("[%d] %s %s\n", job_number, current->command, current->stopped ? "Stopped" : "Running");
        current = current->next;
        job_number++;
    }
}

job_t* find_job(pid_t pid) {
    job_t *current = job_list;
    while (current) {
        if (current->pid == pid) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void update_job_status(pid_t pid, int stopped) {
    job_t *job = find_job(pid);
    if (job) {
        job->stopped = stopped;
    }
}
