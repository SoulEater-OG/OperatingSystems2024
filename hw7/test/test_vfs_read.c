#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "vfs.h"

void read_file(vnode_t *vn, const char *filename) {
    int fd = fat16_open(vn, filename, O_RDONLY);
    if (fd < 0) {
        perror("[DEBUG] Failed to open file for reading");
        return;
    }

    char buffer[BLOCK_SIZE];
    ssize_t bytes_read = fat16_read(vn, buffer, 1, BLOCK_SIZE);
    if (bytes_read < 0) {
        perror("[DEBUG] Failed to read file");
    } else {
        printf("File content of '%s':\n%.*s\n", filename, (int)bytes_read, buffer);
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
        perror("[DEBUG] Failed to open disk image");
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

    printf("[DEBUG] Reading FILE1.TXT\n");
    read_file(&root_vnode, "FILE1.TXT");

    printf("[DEBUG] Reading FILE2.TXT\n");
    read_file(&root_vnode, "FILE2.TXT");

    close(fd);
    return 0;

    printf("[DEBUG] Reading Files Completed Successfully");
}
