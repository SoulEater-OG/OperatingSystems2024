#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

int alarm_triggered = 0;

void alarm_handler(int sig) {
    alarm_triggered = 1;
}

unsigned long get_interrupt_count() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }

    unsigned long interrupts = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strncmp(buffer, "intr", 4) == 0) {
            sscanf(buffer, "intr %lu", &interrupts);
            break;
        }
    }
    fclose(fp);
    return interrupts;
}

void set_interval_timer(long interval) {
    struct itimerval timer;
    timer.it_value.tv_sec = interval;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = interval;
    timer.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error setting timer");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, exit);

    long interval = 1;
    if (argc > 2 && strcmp(argv[1], "-s") == 0) {
        interval = strtol(argv[2], NULL, 10);
        if (interval <= 0) {
            fprintf(stderr, "Invalid interval. Please provide a positive integer.\n");
            exit(EXIT_FAILURE);
        }
    }

    set_interval_timer(interval);

    unsigned long previous_interrupts = get_interrupt_count();
    unsigned long current_interrupts;

    while (1) {
        pause();
        if (alarm_triggered) {
            alarm_triggered = 0;
            current_interrupts = get_interrupt_count();

            printf("Total interrupts since boot: %lu\n", current_interrupts);
            printf("Interrupts in the last %ld seconds: %lu\n\n", interval, current_interrupts - previous_interrupts);
            previous_interrupts = current_interrupts;
        }
    }

    return 0;
}
