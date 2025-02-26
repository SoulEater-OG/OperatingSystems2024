#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "format.h"

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s <filename> [-s <size_mb>]\n", argv[0]);
        exit(1);
    }

    const char *filename = argv[1];
    int size_mb = DEFAULT_DISK_SIZE_MB;

    if (argc == 4 && strcmp(argv[2], "-s") == 0) {
        size_mb = atoi(argv[3]);
    }

    create_disk_image(filename, size_mb);

    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("Error opening disk image");
        exit(1);
    }
    close(fd);

    return 0;
}