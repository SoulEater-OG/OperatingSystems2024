#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defrag.h"

#define DEBUG 1
#define DEBUG_PRINT(fmt, args...) if (DEBUG) printf("[DEBUG] " fmt, ##args)

#define MAX_BLOCKS 1000000  // Define a large enough value for MAX_BLOCKS

// Function to read the superblock from the disk
void read_superblock(FILE *disk, struct superblock *sb) {
    fseek(disk, 512, SEEK_SET); // Move to the start of the superblock (after boot block)
    fread(sb, sizeof(struct superblock), 1, disk);
    DEBUG_PRINT("Superblock read: size=%d, inode_offset=%d, data_offset=%d, free_inode=%d, free_block=%d\n",
                sb->size, sb->inode_offset, sb->data_offset, sb->free_inode, sb->free_block);
}

// Function to read an inode from the disk
void read_inode(FILE *disk, struct superblock *sb, int inode_index, struct inode *node) {
    int inode_position = 1024 + sb->inode_offset * sb->size + inode_index * sizeof(struct inode);
    fseek(disk, inode_position, SEEK_SET);
    fread(node, sizeof(struct inode), 1, disk);
    DEBUG_PRINT("Inode %d read: size=%d, nlink=%d\n", inode_index, node->size, node->nlink);
}

// Function to write an inode to the disk
void write_inode(FILE *disk, struct superblock *sb, int inode_index, struct inode *node) {
    int inode_position = 1024 + sb->inode_offset * sb->size + inode_index * sizeof(struct inode);
    fseek(disk, inode_position, SEEK_SET);
    fwrite(node, sizeof(struct inode), 1, disk);
    DEBUG_PRINT("Inode %d written: size=%d, nlink=%d\n", inode_index, node->size, node->nlink);
}

// Function to copy a block from one location to another
void copy_block(FILE *source_disk, FILE *dest_disk, struct superblock *sb, int source_block, int dest_block) {
    void *block = malloc(sb->size);
    if (!block) {
        perror("malloc");
        exit(1);
    }
    DEBUG_PRINT("Allocated %d bytes for block copy\n", sb->size);
    fseek(source_disk, 1024 + sb->data_offset * sb->size + source_block * sb->size, SEEK_SET);
    fread(block, sb->size, 1, source_disk);
    DEBUG_PRINT("Block read from source: %d\n", source_block);
    fseek(dest_disk, 1024 + sb->data_offset * sb->size + dest_block * sb->size, SEEK_SET);
    fwrite(block, sb->size, 1, dest_disk);
    DEBUG_PRINT("Block written to destination: %d\n", dest_block);
    free(block);
    DEBUG_PRINT("Freed allocated block\n");
}

// Function to read an indirect block
void read_indirect_block(FILE *disk, struct superblock *sb, int block_index, int *blocks, int block_count) {
    fseek(disk, 1024 + sb->data_offset * sb->size + block_index * sb->size, SEEK_SET);
    fread(blocks, block_count * sizeof(int), 1, disk);
    DEBUG_PRINT("Indirect block %d read\n", block_index);
}

// Function to write an indirect block
void write_indirect_block(FILE *disk, struct superblock *sb, int block_index, int *blocks, int block_count) {
    fseek(disk, 1024 + sb->data_offset * sb->size + block_index * sb->size, SEEK_SET);
    fwrite(blocks, block_count * sizeof(int), 1, disk);
    DEBUG_PRINT("Indirect block %d written\n", block_index);
}

// Function to handle and validate indirect blocks
int handle_indirect_blocks(FILE *source_disk, FILE *dest_disk, struct superblock *sb, int block_num, int *current_block, int *all_blocks, int *block_index) {
    int *indirect_blocks = malloc(sb->size);
    if (!indirect_blocks) {
        perror("malloc");
        exit(1);
    }
    DEBUG_PRINT("Allocated %d bytes for indirect block copy\n", sb->size);
    read_indirect_block(source_disk, sb, block_num, indirect_blocks, sb->size / sizeof(int));
    DEBUG_PRINT("Indirect block %d read\n", block_num);

    int new_block_num = *current_block;
    for (int i = 0; i < (int)(sb->size / sizeof(int)); i++) {
        if (indirect_blocks[i] == 0) {
            break; // End of block list
        }
        DEBUG_PRINT("Copying indirect block %d to new location %d\n", indirect_blocks[i], *current_block);
        copy_block(source_disk, dest_disk, sb, indirect_blocks[i], *current_block);
        all_blocks[*block_index] = *current_block;
        (*block_index)++;
        indirect_blocks[i] = (*current_block)++;
        DEBUG_PRINT("Indirect block %d -> %d: %d\n", block_num, indirect_blocks[i], *current_block);
    }

    write_indirect_block(dest_disk, sb, new_block_num, indirect_blocks, sb->size / sizeof(int));
    all_blocks[*block_index] = new_block_num;
    (*block_index)++;
    DEBUG_PRINT("Indirect block %d written\n", new_block_num);

    free(indirect_blocks);
    DEBUG_PRINT("Freed allocated block for indirect block copy\n");

    return new_block_num;
}

// Function to check if the data blocks of a file are contiguous
int is_contiguous(int *dblocks, int num_blocks) {
    for (int i = 0; i < num_blocks - 1; i++) {
        if (dblocks[i] + 1 != dblocks[i + 1]) {
            DEBUG_PRINT("Not Contiguous*** [%d] > [%d]\n", dblocks[i], dblocks[i + 1]);
            return 0; // Not contiguous
        }
    }
    return 1; // Contiguous
}

// Modified move_blocks_contiguous function to use the return value of handle_indirect_blocks
void move_blocks_contiguous(FILE *source_disk, FILE *dest_disk, struct superblock *sb, struct inode *node, int *current_block) {
    int all_blocks[MAX_BLOCKS]; // Allocate more space for all blocks
    int block_index = 0;

    // Move direct blocks to be contiguous
    for (int i = 0; i < N_DBLOCKS && node->dblocks[i] != 0; i++) {
        DEBUG_PRINT("Copying direct block %d to new location %d\n", node->dblocks[i], *current_block);
        copy_block(source_disk, dest_disk, sb, node->dblocks[i], *current_block);
        all_blocks[block_index++] = *current_block;
        node->dblocks[i] = (*current_block)++;
        DEBUG_PRINT("Updated dblock[%d]: %d, current block: %d\n", i, node->dblocks[i], *current_block);
    }

    // Move indirect blocks to be contiguous
    for (int i = 0; i < N_IBLOCKS && node->iblocks[i] != 0; i++) {
        DEBUG_PRINT("Handling indirect block %d\n", node->iblocks[i]);
        node->iblocks[i] = handle_indirect_blocks(source_disk, dest_disk, sb, node->iblocks[i], current_block, all_blocks, &block_index);
        DEBUG_PRINT("Updated iblock[%d]: %d, current block: %d\n", i, node->iblocks[i], *current_block);
    }

    // Move doubly indirect blocks to be contiguous
    if (node->i2block != 0) {
        int *doubly_indirect_blocks = malloc(sb->size);
        if (!doubly_indirect_blocks) {
            perror("malloc");
            exit(1);
        }
        DEBUG_PRINT("Allocated %d bytes for doubly indirect block copy\n", sb->size);
        read_indirect_block(source_disk, sb, node->i2block, doubly_indirect_blocks, sb->size / sizeof(int));
        DEBUG_PRINT("Doubly indirect block %d read\n", node->i2block);

        for (int i = 0; i < (int)(sb->size / sizeof(int)); i++) {
            if (doubly_indirect_blocks[i] == 0) {
                break; // End of block list
            }
            doubly_indirect_blocks[i] = handle_indirect_blocks(source_disk, dest_disk, sb, doubly_indirect_blocks[i], current_block, all_blocks, &block_index);
            DEBUG_PRINT("Updated doubly indirect block[%d]: %d, current block: %d\n", i, doubly_indirect_blocks[i], *current_block);
        }

        int new_doubly_indirect_block = *current_block;
        write_indirect_block(dest_disk, sb, new_doubly_indirect_block, doubly_indirect_blocks, sb->size / sizeof(int));
        node->i2block = new_doubly_indirect_block;
        all_blocks[block_index++] = new_doubly_indirect_block;
        DEBUG_PRINT("New doubly indirect block written at location: %d, current block: %d\n", new_doubly_indirect_block, *current_block);

        (*current_block)++;  // Increment after writing the new doubly indirect block

        free(doubly_indirect_blocks);
        DEBUG_PRINT("Freed allocated block for doubly indirect block copy\n");
        DEBUG_PRINT("Current block after handling doubly indirect blocks: %d\n", *current_block);
    }

    // Move triply indirect blocks to be contiguous
    if (node->i3block != 0) {
        int *triply_indirect_blocks = malloc(sb->size);
        if (!triply_indirect_blocks) {
            perror("malloc");
            exit(1);
        }
        DEBUG_PRINT("Allocated %d bytes for triply indirect block copy\n", sb->size);
        read_indirect_block(source_disk, sb, node->i3block, triply_indirect_blocks, sb->size / sizeof(int));
        DEBUG_PRINT("Triply indirect block %d read\n", node->i3block);

        for (int i = 0; i < (int)(sb->size / sizeof(int)); i++) {
            if (triply_indirect_blocks[i] == 0) {
                break; // End of block list
            }

            // Handle the doubly indirect blocks within this triply indirect block
            triply_indirect_blocks[i] = handle_indirect_blocks(source_disk, dest_disk, sb, triply_indirect_blocks[i], current_block, all_blocks, &block_index);
            DEBUG_PRINT("Updated triply indirect block[%d]: %d, current block: %d\n", i, triply_indirect_blocks[i], *current_block);
        }

        int new_triply_indirect_block = *current_block;
        write_indirect_block(dest_disk, sb, new_triply_indirect_block, triply_indirect_blocks, sb->size / sizeof(int));
        node->i3block = new_triply_indirect_block;
        all_blocks[block_index++] = new_triply_indirect_block;
        DEBUG_PRINT("New triply indirect block written at location: %d, current block: %d\n", new_triply_indirect_block, *current_block);

        (*current_block)++;  // Increment after writing the new triply indirect block

        free(triply_indirect_blocks);
        DEBUG_PRINT("Freed allocated block for triply indirect block copy\n");
        DEBUG_PRINT("Current block after handling triply indirect blocks: %d\n", *current_block);
    }

    // Check if all_blocks are contiguous
    if (!is_contiguous(all_blocks, block_index)) {
        DEBUG_PRINT("Not all blocks are contiguous\n");
    }
}

void defragment_disk(FILE *source_disk, FILE *dest_disk, struct superblock *sb) {
    struct inode node;
    int inode_index = 0;
    int inode_region_size = sb->data_offset * sb->size - sb->inode_offset * sb->size;
    int max_inodes = inode_region_size / sizeof(struct inode);
    int current_block = 0; // Track the next free block in the destination disk

    // Copy boot block and superblock
    void *block = malloc(1024); // Allocate 1024 bytes for boot block and superblock copy
    if (!block) {
        perror("malloc");
        exit(1);
    }
    DEBUG_PRINT("Allocated 1024 bytes for boot and superblock copy\n");
    fseek(source_disk, 0, SEEK_SET);
    fread(block, 1024, 1, source_disk); // Copy boot block (first 512 bytes) and superblock (next 512 bytes)
    DEBUG_PRINT("Boot block and superblock read from source\n");
    fseek(dest_disk, 0, SEEK_SET);
    fwrite(block, 1024, 1, dest_disk);
    DEBUG_PRINT("Boot block and superblock written to destination\n");
    free(block);
    DEBUG_PRINT("Freed allocated block for boot and superblock copy\n");

    int is_empty = 1; // Track if the disk is empty

    // Iterate over all inodes
    for (inode_index = 0; inode_index < max_inodes; inode_index++) {
        read_inode(source_disk, sb, inode_index, &node);

        if (node.nlink == 0) {
            DEBUG_PRINT("Inode %d is free, skipping\n", inode_index);
            // Initialize free inode in destination disk
            memset(&node, 0, sizeof(struct inode));
            write_inode(dest_disk, sb, inode_index, &node);
            continue; // Skip free inode
        }

        is_empty = 0; // Found at least one used inode

        move_blocks_contiguous(source_disk, dest_disk, sb, &node, &current_block);

        // Write the updated inode to the destination disk
        DEBUG_PRINT("Writing updated inode %d to destination disk\n", inode_index);
        write_inode(dest_disk, sb, inode_index, &node);
    }

    if (is_empty) {
        DEBUG_PRINT("Disk is empty. No defragmentation needed.\n");
        return;
    }

    // Update and write the free block list in the superblock
    sb->free_block = current_block; // The next free block starts after the last used block
    fseek(dest_disk, 512, SEEK_SET);
    fwrite(sb, sizeof(struct superblock), 1, dest_disk);
    DEBUG_PRINT("Superblock updated: free_block=%d\n", sb->free_block);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fragmented disk file>\n", argv[0]);
        return 1;
    }

    FILE *source_disk = fopen(argv[1], "rb");
    if (!source_disk) {
        perror("Error opening source disk file");
        return 1;
    }

    struct superblock sb;
    read_superblock(source_disk, &sb);

    char output_filename[256];
    snprintf(output_filename, sizeof(output_filename), "%s-defrag", argv[1]);
    FILE *dest_disk = fopen(output_filename, "wb");
    if (!dest_disk) {
        perror("Error creating destination disk file");
        fclose(source_disk);
        return 1;
    }

    defragment_disk(source_disk, dest_disk, &sb);

    fclose(dest_disk);
    fclose(source_disk);

    printf("Defragmentation completed. Output written to %s\n", output_filename);
    return 0;
}
