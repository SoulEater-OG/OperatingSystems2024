#ifndef FORMAT_H
#define FORMAT_H

#define BLOCK_SIZE 512
#define DEFAULT_DISK_SIZE_MB 1

struct __attribute__((packed)) fat16_boot_sector {
    unsigned char jump[3];
    unsigned char oem[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char num_fats;
    unsigned short root_entries;
    unsigned short total_sectors_short; // if zero, later field is used
    unsigned char media_type;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short num_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_long;
    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned int volume_id;
    unsigned char volume_label[11];
    unsigned char file_system_type[8];
};

typedef struct fat16_entry {
    unsigned char filename[8];
    unsigned char extension[3];
    unsigned char attributes;
    unsigned char reserved;
    unsigned char create_time_tenths;
    unsigned short create_time;
    unsigned short create_date;
    unsigned short last_access_date;
    unsigned short write_time;
    unsigned short write_date;
    unsigned short starting_cluster;
    unsigned int file_size;
} fat16_entry;

void create_disk_image(const char *filename, int size_mb);
void initialize_fat16(int fd, int size_mb);

#endif // FORMAT_H