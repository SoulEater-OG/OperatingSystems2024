#include "format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void create_disk_image(const char *filename, int size_mb) {
    int fd = open(filename, O_CREAT | O_WRONLY, 0666);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Calculate the total number of blocks
    int total_blocks = (size_mb * 1024 * 1024) / BLOCK_SIZE;

    // Initialize the disk with zeros
    char block[BLOCK_SIZE] = {0};
    for (int i = 0; i < total_blocks; ++i) {
        if (write(fd, block, BLOCK_SIZE) != BLOCK_SIZE) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // Initialize the FAT-16 structures
    initialize_fat16(fd, size_mb);

    close(fd);
    printf("Disk image '%s' created with size %d MB\n", filename, size_mb);
}

void initialize_fat16(int fd, int size_mb) {
    // Boot Sector
    struct fat16_boot_sector bs;
    memset(&bs, 0, sizeof(bs));
    memcpy(bs.jump, "\xEB\x3C\x90", 3);  // Boot code jump
    memcpy(bs.oem, "MSDOS5.0", 8);      // OEM Name

    bs.bytes_per_sector = BLOCK_SIZE;
    bs.sectors_per_cluster = 1;
    bs.reserved_sectors = 1;
    bs.num_fats = 2;
    bs.root_entries = 512;               // Max number of root directory entries
    bs.total_sectors_short = (size_mb * 1024 * 1024) / BLOCK_SIZE;
    bs.media_type = 0xF8;                // Fixed disk
    bs.fat_size_sectors = (size_mb * 1024 * 1024) / (BLOCK_SIZE * 256);  // Assuming FAT16
    bs.sectors_per_track = 63;
    bs.num_heads = 255;
    bs.hidden_sectors = 0;
    bs.total_sectors_long = 0;
    bs.drive_number = 0x80;              // Hard disk
    bs.current_head = 0;
    bs.boot_signature = 0x29;
    bs.volume_id = 0x12345678;
    memcpy(bs.volume_label, "NO NAME    ", 11);
    memcpy(bs.file_system_type, "FAT16   ", 8);

        // Convert to little endian manually
    unsigned char buffer[512] = {0};
    memcpy(buffer, bs.jump, 3);
    memcpy(buffer + 3, bs.oem, 8);
    buffer[11] = 0x00;
    buffer[12] = 0x02;
    buffer[13] = bs.sectors_per_cluster;
    buffer[14] = bs.reserved_sectors & 0xFF;
    buffer[15] = (bs.reserved_sectors >> 8) & 0xFF;
    buffer[16] = bs.num_fats;
    buffer[17] = bs.root_entries & 0xFF;
    buffer[18] = (bs.root_entries >> 8) & 0xFF;
    buffer[19] = bs.total_sectors_short & 0xFF;
    buffer[20] = (bs.total_sectors_short >> 8) & 0xFF;
    buffer[21] = bs.media_type;
    buffer[22] = bs.fat_size_sectors & 0xFF;
    buffer[23] = (bs.fat_size_sectors >> 8) & 0xFF;
    buffer[24] = bs.sectors_per_track & 0xFF;
    buffer[25] = (bs.sectors_per_track >> 8) & 0xFF;
    buffer[26] = bs.num_heads & 0xFF;
    buffer[27] = (bs.num_heads >> 8) & 0xFF;
    buffer[28] = bs.hidden_sectors & 0xFF;
    buffer[29] = (bs.hidden_sectors >> 8) & 0xFF;
    buffer[30] = (bs.hidden_sectors >> 16) & 0xFF;
    buffer[31] = (bs.hidden_sectors >> 24) & 0xFF;
    buffer[32] = bs.total_sectors_long & 0xFF;
    buffer[33] = (bs.total_sectors_long >> 8) & 0xFF;
    buffer[34] = (bs.total_sectors_long >> 16) & 0xFF;
    buffer[35] = (bs.total_sectors_long >> 24) & 0xFF;
    buffer[36] = bs.drive_number;
    buffer[37] = bs.current_head;
    buffer[38] = bs.boot_signature;
    buffer[39] = bs.volume_id & 0xFF;
    buffer[40] = (bs.volume_id >> 8) & 0xFF;
    buffer[41] = (bs.volume_id >> 16) & 0xFF;
    buffer[42] = (bs.volume_id >> 24) & 0xFF;
    memcpy(buffer + 43, bs.volume_label, 11);
    memcpy(buffer + 54, bs.file_system_type, 8);

    lseek(fd, 0, SEEK_SET);
    if (write(fd, buffer, sizeof(buffer)) != sizeof(buffer)) {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // FAT Region
    unsigned char fat_entry[3] = {0xF8, 0xFF, 0xFF};  // First two FAT entries
    lseek(fd, BLOCK_SIZE, SEEK_SET);  // Move to the start of the FAT

    for (int i = 0; i < bs.num_fats; ++i) {
        if (write(fd, fat_entry, sizeof(fat_entry)) != sizeof(fat_entry)) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }

        memset(fat_entry, 0, sizeof(fat_entry));  // Reset for next entries

        for (int j = 3; j < bs.fat_size_sectors * BLOCK_SIZE; ++j) {
            if (write(fd, fat_entry, 1) != 1) {
                perror("write");
                close(fd);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Root Directory Region
    struct fat16_entry root_entry = {0};
    lseek(fd, BLOCK_SIZE * (1 + bs.num_fats * bs.fat_size_sectors), SEEK_SET);

    for (int i = 0; i < bs.root_entries; ++i) {
        if (write(fd, &root_entry, sizeof(root_entry)) != sizeof(root_entry)) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }
}