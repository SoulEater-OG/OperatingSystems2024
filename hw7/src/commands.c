#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // Include for file control options
#include <unistd.h> 
#include <string.h>
#include "vfs.h"
#include "commands.h"

#define DEBUG 1 // Set to 1 to enable debug printouts, 0 to disable

void ls_command(vnode_t *vn, char **args) {
    (void)args; // Suppress unused parameter warning
    if (DEBUG) printf("[DEBUG] Opening directory\n");

    int dir_fd = vn->driver->f_opendir(vn, "/");
    if (dir_fd == -1) {
        if (DEBUG) printf("[DEBUG] Failed to open directory.\n");
        return;
    }

    if (DEBUG) printf("[DEBUG] Reading directory entries\n");
    struct dirent *entry;
    while ((entry = vn->driver->f_readdir(vn, dir_fd)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    vn->driver->f_closedir(vn, dir_fd);
    if (DEBUG) printf("[DEBUG] Closed directory\n");
}


void cat_command(vnode_t *vn, char **args) {
    int fd = vn->driver->f_open(vn, args[1], O_RDONLY);
    if (fd == -1) {
        perror("cat open error");
        return;
    }

    char buffer[BLOCK_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = vn->driver->f_read(vn, buffer, sizeof(buffer), 1)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    vn->driver->f_close(vn, fd);
}

void write_command(vnode_t *vn, char **args) {
    if (args[1] == NULL || args[2] == NULL) {
        printf("Usage: write <filename> <data>\n");
        return;
    }

    const char *filename = args[1];
    char data[512] = "";
    strcat(data, args[2]);
    for (int i = 3; args[i] != NULL; i++) {
        strcat(data, " ");
        strcat(data, args[i]);
    }

    printf("[DEBUG] Data to write: %s\n", data);

    int fd = vn->driver->f_open(vn, filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1) {
        perror("write open error");
        return;
    }

    ssize_t bytes_written = vn->driver->f_write(vn, data, strlen(data), 1, filename);
    if (bytes_written == -1) {
        perror("write error");
        vn->driver->f_close(vn, fd);
        return;
    }

    printf("[DEBUG] Written %zd bytes to file: %s\n", bytes_written, data);

    if (vn->driver->f_close(vn, fd) == -1) {
        perror("close error");
    }
}

void read_command(vnode_t *vn, char **args) {
    if (args[1] == NULL) {
        printf("Usage: read <filename>\n");
        return;
    }

    const char *filename = args[1];
    int fd = vn->driver->f_open(vn, filename, O_RDONLY);
    if (fd == -1) {
        perror("read open error");
        return;
    }

    char buffer[512];
    ssize_t bytes_read = vn->driver->f_read(vn, buffer, sizeof(buffer), 1);
    if (bytes_read == -1) {
        perror("read error");
        vn->driver->f_close(vn, fd);
        return;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the string
    printf("Read from file: %s\n", buffer);
    vn->driver->f_close(vn, fd);
}