#include "mem.h"
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>


int m_error = 0;

typedef struct block {
    size_t size;
    struct block *next;
} block;

static block *free_list = NULL;
static char *mem_start = NULL;
static char *mem_end = NULL;

int Mem_Init(int sizeOfRegion) {
    if (sizeOfRegion <= 0) {
        m_error = E_BAD_ARGS;
        return -1;
    }

    size_t total_size = sizeOfRegion + sizeof(block);
    //printf("Mem_Init: Requested size: %d, Total size: %zu\n", sizeOfRegion, total_size);

    mem_start = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem_start == MAP_FAILED) {
        m_error = E_NO_SPACE;
        return -1;
    }

    mem_end = mem_start + total_size;
    free_list = (block *)mem_start;
    free_list->size = sizeOfRegion;
    free_list->next = NULL;

    //printf("Mem_Init: Memory initialized, free_list->size: %zu\n", free_list->size);
    return 0;
}

void *Mem_Alloc(int size) {
    if (size <= 0) {
        m_error = E_BAD_ARGS;
        return NULL;
    }

    size = (size + 7) & ~7;  // 8-byte alignment
    size_t total_size = size + sizeof(block);
    block *curr = free_list;
    block *prev = NULL;

    //printf("Requested size: %d, Total size: %zu\n", size, total_size);

    while (curr) {
        //printf("Current block: %p, Size: %zu\n", (void*)curr, curr->size);
        if (curr->size >= total_size + sizeof(block)) {
            //printf("Found suitable block\n");
            if (curr->size >= total_size + sizeof(block)) {
                block *split = (block *)((char *)curr + total_size);
                split->size = curr->size - total_size;
                split->next = curr->next;
                curr->size = size;
                curr->next = split;
            }
            if (prev)
                prev->next = curr->next;
            else
                free_list = curr->next;
            return (char *)curr + sizeof(block);
        }
        prev = curr;
        curr = curr->next;
    }

    //printf("No suitable block found\n");
    m_error = E_NO_SPACE;
    return NULL;
}

int Mem_Free(void *ptr, int coalesce) {
    if (!ptr)
        return 0;

    block *to_free = (block *)((char *)ptr - sizeof(block));
    if (to_free < (block *)mem_start || to_free >= (block *)mem_end) {
        m_error = E_BAD_POINTER;
        return -1;
    }

    block *curr = free_list;
    block *prev = NULL;

    while (curr && curr < to_free) {
        prev = curr;
        curr = curr->next;
    }

    if (prev)
        prev->next = to_free;
    else
        free_list = to_free;
    to_free->next = curr;

    if (coalesce) {
        if (curr && (char *)to_free + to_free->size + sizeof(block) == (char *)curr) {
            to_free->size += curr->size + sizeof(block);
            to_free->next = curr->next;
        }
        if (prev && (char *)prev + prev->size + sizeof(block) == (char *)to_free) {
            prev->size += to_free->size + sizeof(block);
            prev->next = to_free->next;
        }
    }

    return 0;
}

void Mem_Dump() {
    block *curr = free_list;
    //printf("Free blocks:\n");
    while (curr) {
       //printf("%p: size=%zu\n", (char *)curr + sizeof(block), curr->size);
        curr = curr->next;
    }
}