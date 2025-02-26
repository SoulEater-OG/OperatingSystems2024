#include "matrix_operations.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int **A, **B, **C;
int MAX_ROW_SUM = 0;
pthread_mutex_t maxSumMutex;

void initializeMatrix(int ***matrix) {
    *matrix = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        (*matrix)[i] = (int *)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            (*matrix)[i][j] = 0;
        }
    }
}

void freeMatrix(int ***matrix) {
    for (int i = 0; i < N; i++) {
        free((*matrix)[i]);
    }
    free(*matrix);
}

void *fillMatrix(void *arg) {
    ThreadParam *param = (ThreadParam *)arg;
    int row = param->row;
    int col = param->col;

    unsigned int seed = time(NULL) ^ (row + col + pthread_self());
    A[row][col] = rand_r(&seed) % 10 + 1;
    B[row][col] = rand_r(&seed) % 10 + 1;

    return NULL;
}

void *multiplyMatrix(void *arg) {
    ThreadParam *param = (ThreadParam *)arg;
    int row = param->row;
    int col = param->col;

    C[row][col] = 0;
    for (int k = 0; k < N; k++) {
        C[row][col] += A[row][k] * B[k][col];
    }

    return NULL;
}

void *computeRowSum(void *arg) {
    int row = *(int *)arg;
    int rowSum = 0;

    for (int i = 0; i < N; i++) {
        rowSum += C[row][i];
    }

    pthread_mutex_lock(&maxSumMutex);
    if (rowSum > MAX_ROW_SUM) {
        MAX_ROW_SUM = rowSum;
    }
    pthread_mutex_unlock(&maxSumMutex);

    return NULL;
}

void printMatrix(int **matrix) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d\t", matrix[i][j]);
        }
        printf("\n");
    }
}

void initializeMutex() {
    pthread_mutex_init(&maxSumMutex, NULL);
}
