#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "vfs.h"

void create_and_remove_directory(vnode_t *vn) {
    const char *dir_name = "NEWDIR";
    const char *file_name = "NEWDIR/NEWFILE.TXT";
    const char *file_content = "New file info";

    // Create a new directory
    if (fat16_mkdir(vn, dir_name, 0755) != 0) {
        perror("[DEBUG] Failed to create directory");
        return;
    }

    // Write a file to the new directory
    int fd = fat16_open(vn, file_name, O_WRONLY | O_CREAT);
    if (fd < 0) {
        perror("[DEBUG] Failed to open file for writing");
        return;
    }
    if (fat16_write(vn, file_content, 1, strlen(file_content), file_name) < 0) {
        perror("[DEBUG] Failed to write file");
    }
    fat16_close(vn, fd);

    // List the directory
    printf("[DEBUG] Listing contents of '%s':\n", dir_name);
    fat16_opendir(vn, dir_name);
    struct dirent *dir;
    while ((dir = fat16_readdir(vn, fd)) != NULL) {
        printf("[DEBUG] %s\n", dir->d_name);
    }
    fat16_closedir(vn, fd);

    // Remove the directory
    if (fat16_rmdir(vn, dir_name) != 0) {
        perror("[DEBUG] Failed to remove directory");
        return;
    }
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

    create_and_remove_directory(&root_vnode);

    close(fd);
    return 0;

    printf("[DEBUG] Directory Lifecycle Completed Successfully");
}
