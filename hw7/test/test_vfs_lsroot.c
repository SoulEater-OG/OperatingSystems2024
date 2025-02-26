#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "vfs.h"

void list_directory(vnode_t *vn) {
    struct dirent *dir;
    int dirfd = fat16_opendir(vn, "/");
    if (dirfd < 0) {
        perror("[DEBUG] Failed to open directory");
        return;
    }

    printf("[DEBUG] Directory listing:\n");
    while ((dir = fat16_readdir(vn, dirfd)) != NULL) {
        printf("[DEBUG] %s\n", dir->d_name);
    }

    fat16_closedir(vn, dirfd);
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
        .fs_info.dosfs.fat_ptr = 0 // Root directory starts at cluster 0
    };

    list_directory(&root_vnode);

    close(fd);
    return 0;

    printf("[DEBUG] Listing Root Files Completed Successfully\n");
}
