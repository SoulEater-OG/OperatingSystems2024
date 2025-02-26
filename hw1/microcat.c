#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

void signal_handler(int signum) {
    write(STDERR_FILENO, "Help! I think I’ve been shot!!!\n", 32);
    _exit(EXIT_FAILURE);
}

void concatenate_file(int fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(STDOUT_FILENO, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            write(STDERR_FILENO, "Error writing to stdout\n", 25);
            _exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) {
        write(STDERR_FILENO, "Error reading file\n", 19);
        _exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);

    if (argc == 1) {
        concatenate_file(STDIN_FILENO);
    } else {
        for (int i = 1; i < argc; i++) {
            int fd = open(argv[i], O_RDONLY);
            if (fd < 0) {
                write(STDERR_FILENO, "Error opening file\n", 20);
                continue;
            }

            concatenate_file(fd);
            close(fd);
        }
    }

    return 0;
}
