#ifndef JOB_CONTROL_H
#define JOB_CONTROL_H

#include <sys/types.h>

typedef struct job {
    pid_t pid;
    char command[256];
    int stopped;
    struct job *next;
} job_t;

extern job_t *job_list;

void add_job(pid_t pid, const char *command);
void remove_job(pid_t pid);
void print_jobs();
job_t* find_job(pid_t pid);
void update_job_status(pid_t pid, int stopped);

#endif // JOB_CONTROL_H
