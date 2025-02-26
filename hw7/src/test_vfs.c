#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "vfs.h"

void print_file_content(vnode_t *vn, const char *filename) {
    char buffer[BLOCK_SIZE];
    ssize_t bytes_read;

    int fd = fat16_open(vn, filename, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return;
    }

    while ((bytes_read = fat16_read(vn, buffer, sizeof(char), BLOCK_SIZE, fd)) > 0) {
        fwrite(buffer, sizeof(char), bytes_read, stdout);
    }

    if (bytes_read < 0) {
        perror("Failed to read file");
    }

    fat16_close(vn, fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk_image>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *disk_image = argv[1];
    int fd = open(disk_image, O_RDWR);
    if (fd < 0) {
        perror("Failed to open disk image");
        exit(EXIT_FAILURE);
    }

    printf("[DEBUG] Disk image '%s' opened successfully with fd: %d\n", disk_image, fd);

    vnode_t root_vnode = {
        .vnode_number = 1,
        .name = "/",
        .parent = NULL,
        .permissions = 0755,
        .type = 0,
        .fd = fd,
        .driver = &fat16_driver,
        .fs_info.dosfs.fat_ptr = 2 // Assuming root directory starts at cluster 2
    };

    print_file_content(&root_vnode, "FILE1.TXT");

    close(fd);
    return 0;
}
