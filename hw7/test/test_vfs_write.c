//Code for test_VFS_Write
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "vfs.h"

void print_fat_table(int fd, struct fat16_boot_sector *bs) {
    unsigned short fat_entry;
    off_t fat_offset = bs->reserved_sectors * bs->bytes_per_sector;
    
    printf("[DEBUG] FAT Table:\n");
    for (unsigned int i = 0; i < bs->fat_size_sectors * bs->bytes_per_sector / 2; i++) {
        lseek(fd, fat_offset + i * 2, SEEK_SET);
        read(fd, &fat_entry, sizeof(fat_entry));
        if (fat_entry != 0x0000) {
            printf("[DEBUG] FAT entry %d: 0x%04X\n", i, fat_entry);
        }
    }
}

void write_file(vnode_t *vn, const char *filename, const char *content) {
    int fd = fat16_open(vn, filename, O_WRONLY | O_CREAT);
    if (fd < 0) {
        perror("[DEBUG] Failed to open file for writing");
        return;
    }

    ssize_t bytes_written = fat16_write(vn, content, 1, strlen(content), filename);
    if (bytes_written < 0) {
        perror("[DEBUG] Failed to write file");
    } else {
        printf("[DEBUG] Written %zd bytes to '%s'\n", bytes_written, filename);
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

    write_file(&root_vnode, "FILE1.TXT", "Hello, FAT16!");
    write_file(&root_vnode, "FILE2.TXT", "Goodbye, FAT16!");

    close(fd);
    return 0;
    printf("[DEBUG] Writing Files Completed Successfully");
}