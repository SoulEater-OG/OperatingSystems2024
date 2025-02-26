#ifndef VFS_H
#define VFS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "format.h"

typedef struct vnode {
    int vnode_number;
    char name[255];
    struct vnode *parent;
    int permissions;
    int type;
    int fd; // File descriptor for the disk image
    struct fs_driver *driver;
    union {
        struct unix_fs {
            int inode;
        } unixfs;
        struct dos_fs {
            int fat_ptr;
        } dosfs;
        struct ram_fs {
            unsigned long offset;
        } ramfs;
    } fs_info; // File system specific info
    off_t dir_entry_offset; // Offset for directory entry
} vnode_t;


typedef struct fs_driver {
    int (*f_open)(vnode_t *vn, const char *filename, int flags);
    ssize_t (*f_read)(vnode_t *vn, void *data, size_t size, int num);
    ssize_t (*f_write)(vnode_t *vn, const void *data, size_t size, int num, const char *filename);
    int (*f_close)(vnode_t *vn, int fd);
    int (*f_stat)(vnode_t *vn, const char *filename, struct stat *buf);
    int (*f_remove)(vnode_t *vn, const char *filename);
    int (*f_opendir)(vnode_t *vn, const char *dirname);
    struct dirent *(*f_readdir)(vnode_t *vn, int dirfd);
    int (*f_closedir)(vnode_t *vn, int dirfd);
    int (*f_mkdir)(vnode_t *vn, const char *dirname, mode_t mode);
    int (*f_rmdir)(vnode_t *vn, const char *dirname);
} fs_driver_t;

typedef struct dirent {
    char d_name[255]; // File name
} dirent_t;

// File system functions
int fat16_open(vnode_t *vn, const char *filename, int flags);
ssize_t fat16_read(vnode_t *vn, void *data, size_t size, int num);
ssize_t fat16_write(vnode_t *vn, const void *data, size_t size, int num, const char *filename);
int fat16_close(vnode_t *vn, int fd);
int fat16_stat(vnode_t *vn, const char *filename, struct stat *buf);
int fat16_remove(vnode_t *vn, const char *filename);
int fat16_opendir(vnode_t *vn, const char *dirname);
struct dirent *fat16_readdir(vnode_t *vn, int dirfd);
int fat16_closedir(vnode_t *vn, int dirfd);
int fat16_mkdir(vnode_t *vn, const char *dirname, mode_t mode);
int fat16_rmdir(vnode_t *vn, const char *dirname);
int find_file(vnode_t *vn, const char *filename, struct fat16_entry *entry);
int find_free_root_entry(vnode_t *vn, struct fat16_entry *entry);
int allocate_cluster(vnode_t *vn);
int read_cluster(vnode_t *vn, int cluster, void *buffer);
int write_cluster(vnode_t *vn, int cluster, const void *buffer);
unsigned short get_next_cluster(vnode_t *vn, unsigned short cluster);
void set_next_cluster(vnode_t *vn, unsigned short cluster, unsigned short next_cluster);
off_t cluster_to_offset(vnode_t *vn, unsigned short cluster);

// Declare the FAT-16 driver as an external variable
extern fs_driver_t fat16_driver;

void truncate_file(vnode_t *vn, struct fat16_entry *entry);
void print_hex(unsigned char *data, size_t size);
void fat16_to_tm(unsigned short date, unsigned short time, struct tm *tm);

#endif // VFS_H
