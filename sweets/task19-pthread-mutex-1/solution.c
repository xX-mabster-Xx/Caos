#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { OPS_COUNT = 1000000, THREADS_COUNT = 3 };

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double arr[3];

typedef struct thread_data {
    int number;
    int i1;
    double add1;
    int i2;
    double add2;
} thread_data;

void *modify_data(void *raw_data) {
    thread_data *data = raw_data;

    for (int i = 0; i < OPS_COUNT; ++i) {
        pthread_mutex_lock(&mutex);

        arr[data->i1] += data->add1;
        arr[data->i2] += data->add2;

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t threads[3];
    thread_data all_data[3];

    all_data[0].number = 0;
    all_data[0].i1 = 0;
    all_data[0].add1 = 100;
    all_data[0].i2 = 1;
    all_data[0].add2 = -101;

    all_data[1].number = 1;
    all_data[1].i1 = 1;
    all_data[1].add1 = 200;
    all_data[1].i2 = 2;
    all_data[1].add2 = -201;

    all_data[2].number = 2;
    all_data[2].i1 = 2;
    all_data[2].add1 = 300;
    all_data[2].i2 = 0;
    all_data[2].add2 = -301;

    for (int i = 0; i < THREADS_COUNT; ++i) {
        pthread_create(&threads[i], NULL, modify_data, &all_data[i]);
    }
    for (int i = 0; i < THREADS_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < THREADS_COUNT; ++i) {
        printf("%.10g\n", arr[i]);
    }
}