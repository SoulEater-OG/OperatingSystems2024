#ifndef DEFRAG_H
#define DEFRAG_H

#define N_DBLOCKS 10
#define N_IBLOCKS 4

struct superblock {
    int size;
    int inode_offset;
    int data_offset;
    int swap_offset;
    int free_inode;
    int free_block;
};

struct inode {
    int next_inode;
    int protect;
    int nlink;
    int size;
    int uid;
    int gid;
    int ctime;
    int mtime;
    int atime;
    int dblocks[N_DBLOCKS];
    int iblocks[N_IBLOCKS];
    int i2block;
    int i3block;
};

void read_superblock(FILE *disk, struct superblock *sb);
void read_inode(FILE *disk, struct superblock *sb, int inode_index, struct inode *node);
void write_inode(FILE *disk, struct superblock *sb, int inode_index, struct inode *node);
void copy_block(FILE *source_disk, FILE *dest_disk, struct superblock *sb, int source_block, int dest_block);
void read_indirect_block(FILE *disk, struct superblock *sb, int block_index, int *blocks, int block_count);
void write_indirect_block(FILE *disk, struct superblock *sb, int block_index, int *blocks, int block_count);
void defragment_disk(FILE *source_disk, FILE *dest_disk, struct superblock *sb);

#endif // DEFRAG_H