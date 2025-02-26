#include <stdio.h>
#include <stdlib.h>
#include "matrix_operations.h"
int main() {
    initializeMatrix(&A);
    initializeMatrix(&B);
    initializeMatrix(&C);

    initializeMutex();

    pthread_t threads[N][N];
    ThreadParam params[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            params[i][j].row = i;
            params[i][j].col = j;
            pthread_create(&threads[i][j], NULL, fillMatrix, &params[i][j]);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            pthread_join(threads[i][j], NULL);
        }
    }

    printf("Matrix A:\n");
    printMatrix(A);
    printf("Matrix B:\n");
    printMatrix(B);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            params[i][j].row = i;
            params[i][j].col = j;
            pthread_create(&threads[i][j], NULL, multiplyMatrix, &params[i][j]);
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            pthread_join(threads[i][j], NULL);
        }
    }

    printf("Matrix C:\n");
    printMatrix(C);

    int rowIndices[N];
    pthread_t sumThreads[N];
    for (int i = 0; i < N; i++) {
        rowIndices[i] = i;
        pthread_create(&sumThreads[i], NULL, computeRowSum, &rowIndices[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(sumThreads[i], NULL);
    }

    printf("Maximum Row Sum: %d\n", MAX_ROW_SUM);

    freeMatrix(&A);
    freeMatrix(&B);
    freeMatrix(&C);
    pthread_mutex_destroy(&maxSumMutex);

    return 0;
}
