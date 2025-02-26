#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "vfs.h"

void stat_file(vnode_t *vn, const char *filename) {
    struct stat buf;
    if (fat16_stat(vn, filename, &buf) != 0) {
        perror("[DEBUG] Failed to stat file");
        return;
    }

    printf("[DEBUG] File: %s\n", filename);
    printf("[DEBUG] Size: %lld\n", (long long)buf.st_size);
    printf("[DEBUG] Mode: %o\n", buf.st_mode);
    printf("[DEBUG] Links: %ld\n", (long)buf.st_nlink);
    printf("[DEBUG] Inode: %ld\n", (long)buf.st_ino);
    printf("[DEBUG] Blocks: %lld\n", (long long)buf.st_blocks);
    printf("[DEBUG] Block size: %ld\n", (long)buf.st_blksize);
    printf("[DEBUG] Last modified: %s", ctime(&buf.st_mtime));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk_image> <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *disk_image = argv[1];
    const char *filename = argv[2];
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

    stat_file(&root_vnode, filename);

    close(fd);
    return 0;
}
