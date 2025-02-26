#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_fat16_disk>\n", argv[0]);
        exit(1);
    }

    const char *disk_path = argv[1];
    int disk_fd = open(disk_path, O_RDWR);
    if (disk_fd < 0) {
        perror("open disk image");
        exit(1);
    }

    run_shell(disk_fd);

    close(disk_fd);
    return 0;
}
