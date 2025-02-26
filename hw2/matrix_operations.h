#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <pthread.h>

#define N 4

typedef struct {
    int row;
    int col;
} ThreadParam;

extern int **A, **B, **C;
extern int MAX_ROW_SUM;
extern pthread_mutex_t maxSumMutex;

void *fillMatrix(void *arg);
void *multiplyMatrix(void *arg);
void *computeRowSum(void *arg);
void printMatrix(int **matrix);
void initializeMatrix(int ***matrix);
void freeMatrix(int ***matrix);
void initializeMutex();

#endif
