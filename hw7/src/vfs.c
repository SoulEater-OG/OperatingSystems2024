// CODE FOR VFS.C
#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Function to open a file in the FAT16 file system
int fat16_open(vnode_t *vn, const char *filename, int flags) {
    printf("[DEBUG] Opening file '%s' with flags: %d\n", filename, flags);
    struct fat16_entry entry;

    // Attempt to find the file
    if (find_file(vn, filename, &entry) == 0) {
        vn->fs_info.dosfs.fat_ptr = entry.starting_cluster;
        if ((flags & O_ACCMODE) == O_WRONLY && (flags & O_TRUNC)) {
            // Truncate the file if it exists and O_TRUNC is specified
            truncate_file(vn, &entry);
        }
        vn->dir_entry_offset = lseek(vn->fd, 0, SEEK_CUR) - sizeof(entry); // Store the directory entry offset
        return vn->fd;
    }

    // If O_CREAT flag is specified and file is not found, create a new file entry without allocating a cluster
    if (flags & O_CREAT) {
        printf("[DEBUG] File '%s' not found. Creating new file entry.\n", filename);

        // Prepare a new entry
        memset(&entry, 0, sizeof(entry));

        // Extract the filename and extension
        const char *dot = strrchr(filename, '.');
        size_t name_len = dot ? (size_t)(dot - filename) : strlen(filename);
        size_t ext_len = dot ? strlen(dot + 1) : 0;

        if (name_len > 8) name_len = 8;
        if (ext_len > 3) ext_len = 3;

        strncpy((char *)entry.filename, filename, name_len);
        for (size_t i = name_len; i < 8; i++) {
            entry.filename[i] = ' ';
        }

        if (ext_len > 0) {
            strncpy((char *)entry.extension, dot + 1, ext_len);
        }
        for (size_t i = ext_len; i < 3; i++) {
            entry.extension[i] = ' ';
        }

        entry.attributes = 0x20; // Archive attribute
        entry.starting_cluster = 0; // No cluster allocated yet
        entry.file_size = 0;

        // Find a free directory entry and write the new entry
        off_t dir_offset = find_free_root_entry(vn, &entry);
        if (dir_offset < 0) {
            perror("[DEBUG] Failed to find free root directory entry");
            return -1;
        }
        lseek(vn->fd, dir_offset, SEEK_SET);
        if (write(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
            perror("[DEBUG] Failed to write new directory entry");
            return -1;
        }

        vn->fs_info.dosfs.fat_ptr = 0; // No cluster allocated yet
        vn->dir_entry_offset = dir_offset; // Store directory entry offset
        return vn->fd;
    }

    printf("[DEBUG] File '%s' not found and O_CREAT not specified.\n", filename);
    return -1;
}

// Function to read data from a file in the FAT16 file system
ssize_t fat16_read(vnode_t *vn, void *data, size_t size, int num) {
    ssize_t total_bytes_read = 0;
    char buffer[BLOCK_SIZE];

    while (vn->fs_info.dosfs.fat_ptr < 0xFFF8 && total_bytes_read < (ssize_t)(size * num)) {
        off_t cluster_offset = cluster_to_offset(vn, vn->fs_info.dosfs.fat_ptr);
        lseek(vn->fd, cluster_offset, SEEK_SET);
        ssize_t bytes_read = read(vn->fd, buffer, BLOCK_SIZE);
        if (bytes_read <= 0) {
            break;
        }

        ssize_t bytes_to_copy = size * num - total_bytes_read;
        bytes_to_copy = bytes_to_copy > bytes_read ? bytes_read : bytes_to_copy;
        memcpy((char*)data + total_bytes_read, buffer, bytes_to_copy);
        total_bytes_read += bytes_to_copy;

        unsigned short next_cluster = get_next_cluster(vn, vn->fs_info.dosfs.fat_ptr);
        if (next_cluster >= 0xFFF8 || next_cluster <= 0) {
            vn->fs_info.dosfs.fat_ptr = 0xFFFF;
            break;
        }
        
        vn->fs_info.dosfs.fat_ptr = next_cluster;  // Update to next cluster
    }

    return total_bytes_read;
}

ssize_t fat16_write(vnode_t *vn, const void *data, size_t size, int num, const char *filename) {
    struct fat16_entry entry;
    memset(&entry, 0, sizeof(struct fat16_entry));

    // Use the stored directory entry offset to find the directory entry
    lseek(vn->fd, vn->dir_entry_offset, SEEK_SET);
    if (read(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("[DEBUG] Failed to read directory entry");
        return -1;
    }

    // Allocate the first cluster if it hasn't been allocated yet
    int first_cluster = entry.starting_cluster;
    if (first_cluster == 0) {
        first_cluster = allocate_cluster(vn);
        if (first_cluster == -1) {
            printf("[DEBUG] Failed to allocate the first cluster\n");
            return -1;
        }
        entry.starting_cluster = first_cluster;
        // Set the FAT entry for the new cluster
        set_next_cluster(vn, first_cluster, 0xFFF8);
    }

    // Prepare to write data to the cluster
    off_t cluster_offset = cluster_to_offset(vn, first_cluster);
    if (cluster_offset == -1) {
        printf("[DEBUG] Failed to convert cluster number to offset\n");
        return -1;
    }

    // Write the actual file content to the disk at the calculated cluster offset
    lseek(vn->fd, cluster_offset, SEEK_SET);
    ssize_t bytes_to_write = size * num;
    ssize_t bytes_written = write(vn->fd, data, bytes_to_write);
    if (bytes_written != (ssize_t)bytes_to_write) {
        perror("[DEBUG] Failed to write file data");
        return -1;
    }

    // Properly terminate the cluster chain by setting the next cluster value to 0xFFF8 (EOF)
    set_next_cluster(vn, first_cluster, 0xFFF8);

    // Update the directory entry for the new file
    const char *dot = strrchr(filename, '.');
    size_t name_len = dot ? (size_t)(dot - filename) : strlen(filename);
    size_t ext_len = dot ? strlen(dot + 1) : 0;

    if (name_len > 8) name_len = 8;
    if (ext_len > 3) ext_len = 3;

    strncpy((char *)entry.filename, filename, name_len);
    for (size_t i = name_len; i < 8; i++) {
        entry.filename[i] = ' ';
    }

    if (ext_len > 0) {
        strncpy((char *)entry.extension, dot + 1, ext_len);
    }
    for (size_t i = ext_len; i < 3; i++) {
        entry.extension[i] = ' ';
    }

    entry.attributes = 0x20; // Archive attribute
    entry.starting_cluster = first_cluster;
    entry.file_size = bytes_written;

    // Debug printout to validate entry information
    printf("[DEBUG] Directory entry information:\n");
    printf("[DEBUG] Filename: '%.8s'\n", entry.filename);
    printf("[DEBUG] Extension: '%.3s'\n", entry.extension);
    printf("[DEBUG] Attributes: 0x%02X\n", entry.attributes);
    printf("[DEBUG] Starting cluster: %d\n", entry.starting_cluster);
    printf("[DEBUG] File size: %u\n", entry.file_size);

    // Write the updated directory entry back to its original position
    lseek(vn->fd, vn->dir_entry_offset, SEEK_SET);
    if (write(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("[DEBUG] Failed to write updated directory entry");
        return -1;
    }

    printf("[DEBUG] Successfully written %zd bytes to '%s' at cluster %d\n", bytes_written, filename, first_cluster);
    return bytes_written;
}



// Function to close a file in the FAT16 file system
int fat16_close(vnode_t *vn, int fd) {
    printf("[DEBUG] Closing file with fd: %d\n", fd);
    (void)vn;
    (void)fd;
    return 0;
}

int fat16_remove(vnode_t *vn, const char *filename) {
    struct fat16_entry entry;
    off_t entry_offset;

    // Find the file entry
    entry_offset = find_file(vn, filename, &entry);
    if (entry_offset < 0) {
        printf("[DEBUG] File '%s' not found.\n", filename);
        return -1;
    }

    // Check if the entry is a file
    if (entry.attributes & 0x10) {
        printf("[DEBUG] '%s' is a directory, not a file.\n", filename);
        return -1;
    }

    // Free the clusters used by the file
    unsigned short cluster = entry.starting_cluster;
    unsigned short next_cluster;
    while (cluster < 0xFFF8) {
        next_cluster = get_next_cluster(vn, cluster);
        set_next_cluster(vn, cluster, 0x0000);
        cluster = next_cluster;
    }

    // Mark the file entry as deleted
    entry.filename[0] = 0xE5;
    lseek(vn->fd, entry_offset, SEEK_SET);
    if (write(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("[DEBUG] Failed to mark file entry as deleted");
        return -1;
    }

    printf("[DEBUG] File '%s' removed\n", filename);
    return 0;
}

int fat16_stat(vnode_t *vn, const char *filename, struct stat *buf) {
    struct fat16_entry entry;
    off_t entry_offset;

    // Find the file entry
    entry_offset = find_file(vn, filename, &entry);
    if (entry_offset < 0) {
        printf("[DEBUG] File '%s' not found.\n", filename);
        return -1;
    }

    // Clear the stat structure
    memset(buf, 0, sizeof(struct stat));

    // Set file type and mode
    if (entry.attributes & 0x10) {
        buf->st_mode = S_IFDIR | 0755;
    } else {
        buf->st_mode = S_IFREG | 0644;
    }

    // Set file size
    buf->st_size = entry.file_size;

    // Set number of links (1 for files, 2 for directories)
    buf->st_nlink = (entry.attributes & 0x10) ? 2 : 1;

    // Set inode number to the starting cluster (unique within the FAT16 volume)
    buf->st_ino = entry.starting_cluster;

    // Set timestamps
    struct tm tm;
    fat16_to_tm(entry.write_date, entry.write_time, &tm);
    buf->st_mtime = mktime(&tm);
    buf->st_ctime = buf->st_mtime;
    buf->st_atime = buf->st_mtime;

    // Set block size and number of blocks
    buf->st_blksize = 512;
    buf->st_blocks = (buf->st_size + 511) / 512;

    return 0;
}

int fat16_opendir(vnode_t *vn, const char *dirname) {
    struct fat16_boot_sector bs;

    // Ensure the root directory is being opened
    if (strcmp(dirname, "/") != 0) {
        printf("[DEBUG] Only the root directory can be opened in this example.\n");
        return -1;
    }

    // Read the boot sector to get the root directory offset
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("[DEBUG] Failed to read boot sector");
        return -1;
    }

    off_t root_dir_offset = (bs.reserved_sectors + bs.num_fats * bs.fat_size_sectors) * bs.bytes_per_sector;
    lseek(vn->fd, root_dir_offset, SEEK_SET);
    vn->fs_info.dosfs.fat_ptr = root_dir_offset; // Root directory starts at this offset

    return vn->fd;
}

struct dirent *fat16_readdir(vnode_t *vn, int dirfd) {
    (void)dirfd;
    static struct dirent dir_entry;
    struct fat16_entry entry;

    while (1) {
        // Read the next directory entry
        if (read(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
            return NULL; // End of directory or error
        }

        // Check if the entry is valid
        if (entry.filename[0] == 0x00) {
            return NULL; // No more entries
        }
        if (entry.filename[0] == 0xE5) {
            continue; // Skip deleted entries
        }

        // Convert the FAT16 directory entry to a struct dirent
        char name[9] = {0};
        char ext[4] = {0};
        strncpy(name, (char *)entry.filename, 8);
        strncpy(ext, (char *)entry.extension, 3);

        // Remove trailing spaces from the filename and extension
        for (int i = 7; i >= 0; i--) {
            if (name[i] == ' ') {
                name[i] = '\0';
            } else {
                break;
            }
        }
        for (int i = 2; i >= 0; i--) {
            if (ext[i] == ' ') {
                ext[i] = '\0';
            } else {
                break;
            }
        }

        // Combine filename and extension
        if (ext[0] != '\0') {
            snprintf(dir_entry.d_name, sizeof(dir_entry.d_name), "%s.%s", name, ext);
        } else {
            snprintf(dir_entry.d_name, sizeof(dir_entry.d_name), "%s", name);
        }

        return &dir_entry;
    }
}


int fat16_closedir(vnode_t *vn, int dirfd) {
    // In this example, there's nothing specific to do to close the directory
    (void)vn;
    (void)dirfd;
    return 0;
}

int fat16_mkdir(vnode_t *vn, const char *dirname, mode_t mode) {
    (void)mode;
    struct fat16_entry entry;

    // Check if the directory already exists
    if (find_file(vn, dirname, &entry) == 0) {
        printf("[DEBUG] Directory '%s' already exists.\n", dirname);
        return -1;
    }

    // Prepare a new directory entry
    memset(&entry, 0, sizeof(entry));
    strncpy((char *)entry.filename, dirname, 8);
    for (int i = strlen(dirname); i < 8; i++) {
        entry.filename[i] = ' ';
    }
    memset(entry.extension, ' ', 3);
    entry.attributes = 0x10; // Directory attribute

    // Find a free directory entry
    off_t dir_offset = find_free_root_entry(vn, &entry);
    if (dir_offset < 0) {
        perror("[DEBUG] Failed to find free root directory entry");
        return -1;
    }

    // Allocate the first cluster for the directory
    int first_cluster = allocate_cluster(vn);
    if (first_cluster == -1) {
        printf("[DEBUG] Failed to allocate the first cluster for the directory\n");
        return -1;
    }
    entry.starting_cluster = first_cluster;

    // Write the directory entry to the root directory
    lseek(vn->fd, dir_offset, SEEK_SET);
    if (write(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("[DEBUG] Failed to write new directory entry");
        return -1;
    }

    printf("[DEBUG] Directory '%s' created with cluster %d\n", dirname, first_cluster);
    return 0;
}

int fat16_rmdir(vnode_t *vn, const char *dirname) {
    struct fat16_entry entry;

    // Find the directory entry
    if (find_file(vn, dirname, &entry) != 0) {
        printf("[DEBUG] Directory '%s' not found.\n", dirname);
        return -1;
    }

    // Check if the entry is a directory
    if (!(entry.attributes & 0x10)) {
        printf("[DEBUG] '%s' is not a directory.\n", dirname);
        return -1;
    }

    // Traverse the directory and remove all files and subdirectories
    off_t dir_offset = cluster_to_offset(vn, entry.starting_cluster);
    lseek(vn->fd, dir_offset, SEEK_SET);

    struct fat16_entry sub_entry;
    while (read(vn->fd, &sub_entry, sizeof(sub_entry)) == sizeof(sub_entry)) {
        if (sub_entry.filename[0] == 0x00) {
            break; // End of directory
        }
        if (sub_entry.filename[0] == 0xE5) {
            continue; // Skip deleted entries
        }

        // Construct the full path of the subdirectory entry
        char sub_name[12] = {0};
        strncpy(sub_name, (char *)sub_entry.filename, 8);
        sub_name[8] = '\0';
        for (int i = 7; i >= 0; i--) {
            if (sub_name[i] == ' ') {
                sub_name[i] = '\0';
            } else {
                break;
            }
        }
        strcat(sub_name, ".");
        strncat(sub_name, (char *)sub_entry.extension, 3);

        // Remove the subdirectory entry
        if (sub_entry.attributes & 0x10) {
            fat16_rmdir(vn, sub_name);
        } else {
            fat16_remove(vn, sub_name);
        }
    }

    // Mark the directory entry as deleted
    entry.filename[0] = 0xE5;
    lseek(vn->fd, vn->dir_entry_offset, SEEK_SET);
    if (write(vn->fd, &entry, sizeof(entry)) != sizeof(entry)) {
        perror("[DEBUG] Failed to mark directory entry as deleted");
        return -1;
    }

    printf("[DEBUG] Directory '%s' removed\n", dirname);
    return 0;
}

int find_file(vnode_t *vn, const char *filename, struct fat16_entry *entry) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("Failed to read boot sector\n");
        return -1;
    }

    off_t root_dir_offset = (bs.reserved_sectors + bs.num_fats * bs.fat_size_sectors) * bs.bytes_per_sector;
    lseek(vn->fd, root_dir_offset, SEEK_SET);
    printf("[DEBUG] Root directory offset: %ld\n", root_dir_offset);

    for (unsigned int i = 0; i < bs.root_entries; i++) {
        if (read(vn->fd, entry, sizeof(*entry)) != sizeof(*entry)) {
            perror("Failed to read directory entry\n");
            return -1;
        }

        if (entry->filename[0] == 0x00) {
            break;
        }

        char name[9] = {0};
        char ext[4] = {0};
        strncpy(name, (char*)entry->filename, 8);
        strncpy(ext, (char*)entry->extension, 3);
        for (int j = 7; j >= 0; j--) if (name[j] == ' ') name[j] = '\0';
        for (int j = 2; j >= 0; j--) if (ext[j] == ' ') ext[j] = '\0';

        char full_name[13];
        sprintf(full_name, "%s.%s", name, ext);
        printf("[DEBUG] Comparing '%s' with '%s'\n", full_name, filename);

        if (strcasecmp(full_name, filename) == 0) {
            printf("[DEBUG] File '%s' found in root directory.\n", filename);
            return 0;
        }
    }

    printf("[DEBUG] File '%s' not found.\n", filename);
    return -1;
}

int find_free_root_entry(vnode_t *vn, struct fat16_entry *entry) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("[DEBUG] Failed to read boot sector");
        return -1;
    }

    off_t root_dir_offset = (bs.reserved_sectors + bs.num_fats * bs.fat_size_sectors) * bs.bytes_per_sector;
    lseek(vn->fd, root_dir_offset, SEEK_SET);

    for (unsigned int i = 0; i < bs.root_entries; i++) {
        if (read(vn->fd, entry, sizeof(*entry)) != sizeof(*entry)) {
            perror("[DEBUG] Failed to read directory entry");
            return -1;
        }

        if (entry->filename[0] == 0x00 || entry->filename[0] == 0xE5) {
            lseek(vn->fd, -sizeof(*entry), SEEK_CUR);
            printf("[DEBUG] Free root directory entry found at offset: %ld\n", lseek(vn->fd, 0, SEEK_CUR));
            return lseek(vn->fd, 0, SEEK_CUR);
        }
    }

    printf("[DEBUG] No free root directory entries found\n");
    return -1;
}

// Function to allocate a free cluster in the FAT16 file system
int allocate_cluster(vnode_t *vn) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("[DEBUG] Failed to read boot sector");
        return -1;
    }

    off_t fat_offset = bs.reserved_sectors * bs.bytes_per_sector;
    unsigned short fat_entry;
    unsigned int cluster_count = bs.total_sectors_short ? bs.total_sectors_short : bs.total_sectors_long;
    cluster_count /= bs.sectors_per_cluster;

    for (unsigned int i = 2; i < cluster_count; i++) {
        lseek(vn->fd, fat_offset + i * 2, SEEK_SET);
        if (read(vn->fd, &fat_entry, sizeof(fat_entry)) != sizeof(fat_entry)) {
            perror("[DEBUG] Failed to read FAT entry");
            return -1;
        }

        if (fat_entry == 0x0000) {
            fat_entry = 0xFFF8; // Mark as end of file
            lseek(vn->fd, fat_offset + i * 2, SEEK_SET);
            if (write(vn->fd, &fat_entry, sizeof(fat_entry)) != sizeof(fat_entry)) {
                perror("[DEBUG] Failed to write FAT entry");
                return -1;
            }
            printf("[DEBUG] Allocated cluster %d and set to 0xFFF8\n", i);
            return i;
        }
    }

    printf("[DEBUG] No free clusters found\n");
    return -1;
}

int read_cluster(vnode_t *vn, int cluster, void *buffer) {
    off_t cluster_offset = cluster_to_offset(vn, cluster);
    lseek(vn->fd, cluster_offset, SEEK_SET);
    ssize_t bytes_read = read(vn->fd, buffer, BLOCK_SIZE);
    printf("[DEBUG] Read cluster %d. Bytes read: %zd\n", cluster, bytes_read);
    return bytes_read;
}

int write_cluster(vnode_t *vn, int cluster, const void *buffer) {
    off_t cluster_offset = cluster_to_offset(vn, cluster);
    lseek(vn->fd, cluster_offset, SEEK_SET);
    ssize_t bytes_written = write(vn->fd, buffer, BLOCK_SIZE);
    printf("[DEBUG] Write cluster %d. Bytes written: %zd\n", cluster, bytes_written);
    return bytes_written;
}

unsigned short get_next_cluster(vnode_t *vn, unsigned short cluster) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("[DEBUG] Failed to read boot sector");
        return 0xFFFF;  // Indicate error
    }

    off_t fat_offset = bs.reserved_sectors * bs.bytes_per_sector + cluster * 2;
    lseek(vn->fd, fat_offset, SEEK_SET);

    unsigned short next_cluster;
    if (read(vn->fd, &next_cluster, sizeof(next_cluster)) != sizeof(next_cluster)) {
        perror("[DEBUG] Failed to read FAT entry");
        return 0xFFFF;  // Indicate error
    }

    printf("[DEBUG] FAT offset for cluster %d: %ld\n", cluster, fat_offset);
    printf("[DEBUG] Current cluster: %d, Next cluster: %d\n", cluster, next_cluster);

    if (next_cluster >= 0xFFF8) {
        return 0xFFFF;  // EOF
    }
    return next_cluster;
}


void set_next_cluster(vnode_t *vn, unsigned short cluster, unsigned short next_cluster) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("[DEBUG] Failed to read boot sector");
        return;
    }

    off_t fat_offset = bs.reserved_sectors * bs.bytes_per_sector + cluster * 2;
    lseek(vn->fd, fat_offset, SEEK_SET);
    if (write(vn->fd, &next_cluster, sizeof(next_cluster)) != sizeof(next_cluster)) {
        perror("[DEBUG] Failed to write FAT entry");
        return;
    }

    printf("[DEBUG] Set next cluster for cluster %d to %d (0x%X)\n", cluster, next_cluster, next_cluster);
}


// Function to convert a cluster number to a file offset in the FAT16 file system
off_t cluster_to_offset(vnode_t *vn, unsigned short cluster) {
    struct fat16_boot_sector bs;
    lseek(vn->fd, 0, SEEK_SET);
    if (read(vn->fd, &bs, sizeof(bs)) != sizeof(bs)) {
        perror("Failed to read boot sector");
        return -1;
    }
    
    off_t data_offset = (bs.reserved_sectors + bs.num_fats * bs.fat_size_sectors + (bs.root_entries * sizeof(struct fat16_entry)) / bs.bytes_per_sector) * bs.bytes_per_sector;
    data_offset += (cluster - 2) * bs.sectors_per_cluster * bs.bytes_per_sector;
    return data_offset;
}

void truncate_file(vnode_t *vn, struct fat16_entry *entry) {
    // Function to truncate the file and update the FAT
    set_next_cluster(vn, entry->starting_cluster, 0xFFF8);
    entry->file_size = 0;
    // Update directory entry to reflect new size
    off_t dir_offset = find_free_root_entry(vn, entry);
    lseek(vn->fd, dir_offset, SEEK_SET);
    write(vn->fd, entry, sizeof(*entry));
}

void print_hex(unsigned char *data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void fat16_to_tm(unsigned short date, unsigned short time, struct tm *tm) {
    tm->tm_year = ((date >> 9) & 0x7F) + 80;
    tm->tm_mon = ((date >> 5) & 0xF) - 1;
    tm->tm_mday = date & 0x1F;
    tm->tm_hour = (time >> 11) & 0x1F;
    tm->tm_min = (time >> 5) & 0x3F;
    tm->tm_sec = (time & 0x1F) * 2;
    tm->tm_isdst = -1;
}

fs_driver_t fat16_driver = {
    .f_open = fat16_open,
    .f_read = fat16_read,
    .f_write = fat16_write,
    .f_close = fat16_close,
    .f_stat = NULL,
    .f_remove = fat16_remove,
    .f_opendir = fat16_opendir,
    .f_readdir = fat16_readdir,
    .f_closedir = fat16_closedir,
    .f_mkdir = fat16_mkdir,
    .f_rmdir = fat16_rmdir
};