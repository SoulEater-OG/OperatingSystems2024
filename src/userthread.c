#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <valgrind/valgrind.h>
#include <unistd.h>
#include <pthread.h>
#include "userthread.h"

#define MAX_THREADS 100
#define DEBUG 1

typedef struct {
    int tid;
    ucontext_t context;
    int priority;
    int state;
    int wait_on_tid;
    int ticks;
    int last_run_time;
    int total_run_time;
    int num_runs;
    void *stack;
    int valgrind_stack_id;
    pthread_cond_t cond;
} tcb_t;

enum { FINISHED = 0, INITIALIZED = 1, RUNNING = 2, WAITING = 3 };


static tcb_t threads[MAX_THREADS];
static int num_threads = 0;
static int current_thread = -1;
static int scheduling_policy = FIFO;
static int ticks = 0;
static FILE *log_file;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static ucontext_t main_context;

void print_thread_states() {
    printf("[DEBUG] Current thread states:\n");
    for (int i = 0; i < num_threads; i++) {
        const char *state;
        switch(threads[i].state) {
            case FINISHED: state = "FINISHED"; break;
            case INITIALIZED: state = "INITIALIZED"; break;
            case RUNNING: state = "RUNNING"; break;
            case WAITING: state = "WAITING"; break;
            default: state = "UNKNOWN"; break;
        }
        printf("Thread %d: %s\n", threads[i].tid, state);
    }
}

static void wrapper(void *(*start_routine)(void *), void *arg) {
    if(DEBUG) printf("[DEBUG] Thread %d started execution\n", threads[current_thread].tid);
    start_routine(arg);

    pthread_mutex_lock(&mutex);
    if(DEBUG) printf("[DEBUG] Thread %d finished execution\n", threads[current_thread].tid);
    threads[current_thread].state = FINISHED;
    pthread_cond_broadcast(&threads[current_thread].cond);
    pthread_mutex_unlock(&mutex);

    thread_yield();
}

static void scheduler(int sig) {
    (void) sig;
    pthread_mutex_lock(&mutex);

    if (DEBUG) printf("[DEBUG] Scheduler invoked, current_thread = %d, ticks = %d\n", current_thread+1, ticks);
    if (DEBUG) print_thread_states();
    ticks++;

    int next_thread = -1;

    for (int i = 0; i < num_threads; i++) {
        if (threads[i].state == WAITING) {
            int wait_on_tid = threads[i].wait_on_tid;
            if (threads[wait_on_tid - 1].state == FINISHED) {
                threads[i].state = INITIALIZED;
            }
        }
    }

    if (scheduling_policy == FIFO) {
        for (int i = 1; i <= num_threads; i++) {
            int idx = (current_thread + i) % num_threads;
            if (threads[idx].state == INITIALIZED) {
                next_thread = idx;
                break;
            }
        }
    } else if (scheduling_policy == SJF) {
        int min_ticks = INT_MAX;
        for (int i = 0; i < num_threads; i++) {
            if (threads[i].state == INITIALIZED && threads[i].ticks < min_ticks) {
                min_ticks = threads[i].ticks;
                next_thread = i;
            }
        }
    } else if (scheduling_policy == PRIORITY) {
        int max_priority = INT_MIN;
        for (int i = 0; i < num_threads; i++) {
            if (threads[i].state == INITIALIZED && threads[i].priority > max_priority) {
                max_priority = threads[i].priority;
                next_thread = i;
            }
        }
    }

    if (next_thread == -1) {
        if (DEBUG) printf("[DEBUG] No runnable threads found. Returning to main context.\n");
        pthread_mutex_unlock(&mutex);
        if (current_thread != -1) {
            swapcontext(&threads[current_thread].context, &main_context);
        } else {
            setcontext(&main_context);
        }
        return;
    }

    if (DEBUG) printf("[DEBUG] Context switch from Thread %d to Thread %d\n", current_thread+1, next_thread+1);
    current_thread = next_thread;
    threads[next_thread].state = RUNNING;
    pthread_mutex_unlock(&mutex);
    setcontext(&threads[next_thread].context);
}

static void setup_timer() {
    struct itimerval timer;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, scheduler);
}

int thread_libinit(int policy) {
    if (policy != FIFO && policy != SJF && policy != PRIORITY) {
        fprintf(stderr, "Invalid scheduling policy\n");
        return -1;
    }

    scheduling_policy = policy;
    log_file = fopen("userthread_log.txt", "w");
    if (log_file == NULL) {
        perror("Error opening log file");
        return -1;
    }

    if (DEBUG) printf("[DEBUG] Thread library initialized with policy %d\n", policy);
    getcontext(&main_context);
    setup_timer();
    return 0;
}

int thread_libterminate(void) {
    pthread_mutex_lock(&mutex);

    if (DEBUG) printf("[DEBUG] Thread library terminating\n");

    for (int i = 0; i < num_threads; i++) {
        tcb_t *tcb = &threads[i];
        if (DEBUG) printf("[DEBUG] Thread %d state: %d\n", i, tcb->state);
        tcb->state = FINISHED;
        VALGRIND_STACK_DEREGISTER(tcb->valgrind_stack_id);
        free(tcb->stack);
    }

    num_threads = 0;
    current_thread = -1;
    pthread_mutex_unlock(&mutex);
    if (DEBUG) printf("[DEBUG] Thread library terminated\n");
    fclose(log_file);
    return 0;
}

int thread_create(void (*func)(void *), void *arg, int priority) {
    pthread_mutex_lock(&mutex);
    if (num_threads >= MAX_THREADS) {
        fprintf(log_file, "Maximum number of threads reached\n");
        fflush(log_file);
        if (DEBUG) printf("[DEBUG] Maximum number of threads reached\n");
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    tcb_t *tcb = &threads[num_threads];
    if (tcb->stack != NULL) {
        VALGRIND_STACK_DEREGISTER(tcb->valgrind_stack_id);
        free(tcb->stack);
    }

    memset(tcb, 0, sizeof(tcb_t));
    tcb->tid = num_threads + 1;
    tcb->wait_on_tid = -1;
    tcb->priority = priority;
    tcb->state = INITIALIZED;
    tcb->stack = malloc(STACKSIZE);
    if (tcb->stack == NULL) {
        perror("Failed to allocate stack");
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    getcontext(&tcb->context);
    tcb->context.uc_stack.ss_sp = tcb->stack;
    tcb->context.uc_stack.ss_size = STACKSIZE;
    tcb->context.uc_link = &main_context;
    tcb->valgrind_stack_id = VALGRIND_STACK_REGISTER(tcb->stack, tcb->stack + STACKSIZE);
    makecontext(&tcb->context, (void (*)())wrapper, 2, func, arg);

    if (DEBUG) printf("[DEBUG] Thread %d initialized successfully with priority %d\n", tcb->tid, priority);

    num_threads++;

    pthread_mutex_unlock(&mutex);
    return tcb->tid;
}

int thread_yield(void) {
    if (current_thread == -1) {
        if (DEBUG) printf("[DEBUG] No thread is currently running\n");
        return 0;
    }

    if (DEBUG) printf("[DEBUG] Thread %d yielding\n", current_thread+1);
    raise(SIGALRM);
    return 0;
}


int thread_join(int tid) {
    pthread_mutex_lock(&mutex);
    if (tid <= 0 || tid > num_threads) {
        fprintf(log_file, "Invalid thread ID: %d\n", tid);
        fflush(log_file);
        if (DEBUG) printf("[DEBUG] Invalid thread ID: %d\n", tid);
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    int index = tid - 1;
    tcb_t *tcb = &threads[index];

    if (tcb->state == FINISHED) {
        if (DEBUG) printf("[DEBUG] Thread %d already joined\n", tid);
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    if (current_thread != -1 && tid > 0 && tid <= num_threads) {
        if(threads[current_thread].state != FINISHED) {    
            threads[current_thread].state = WAITING;
            threads[current_thread].wait_on_tid = tid;
        }
    }

    while (tcb->state != FINISHED) {
        pthread_cond_wait(&tcb->cond, &mutex);
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}
