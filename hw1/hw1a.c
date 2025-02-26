#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h> 

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

int main() {
    signal(SIGALRM, alarm_handler);
    signal(SIGINT, exit);

    unsigned long previous_interrupts = get_interrupt_count();
    unsigned long current_interrupts;

    alarm(1);

    while (1) {
        pause();
        if (alarm_triggered) {
            alarm_triggered = 0;
            current_interrupts = get_interrupt_count();

            printf("Total interrupts since boot: %lu\n", current_interrupts);
            printf("Interrupts in the last second: %lu\n\n", current_interrupts - previous_interrupts);
            previous_interrupts = current_interrupts;

            alarm(1);
        }
    }

    return 0;
}
