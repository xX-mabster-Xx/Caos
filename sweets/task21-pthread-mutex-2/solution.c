#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_cond_t found_new_prime = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

enum { THREADS_COUNT = 10 };

uint64_t prime = 0;

typedef struct thread_data {
    uint32_t iterations;
    int32_t acc_idx[2];
    double payment[2];
    double *accs;
    pthread_mutex_t *mutexes;
} thread_data;

void *banking_staff(void *raw_data) {
    thread_data *data = raw_data;

    for (uint32_t i = 0; i < data->iterations; ++i) {
        while (true) {
            pthread_mutex_lock(&data->mutexes[data->acc_idx[0]]);
            if (pthread_mutex_trylock(&data->mutexes[data->acc_idx[1]]) == 0) {
                break;
            } else {
                pthread_mutex_unlock(&data->mutexes[data->acc_idx[0]]);
            }
        }
        for (int i = 0; i < 2; ++i) {
            data->accs[data->acc_idx[i]] += data->payment[i];
        }
        for (int i = 0; i < 2; ++i) {
            pthread_mutex_unlock(&data->mutexes[data->acc_idx[i]]);
        }
    }

    return NULL;
}

int main() {
    uint32_t acc_count = 0;
    uint32_t thr_count = 0;

    if (scanf("%" PRIu32 " %" PRIu32, &acc_count, &thr_count) <= 0) {
        perror("scanf");
        return EXIT_FAILURE;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setguardsize(&attr, 0);
    pthread_attr_setstacksize(&attr, sysconf(_SC_THREAD_STACK_MIN));

    pthread_t *threads = calloc(thr_count, sizeof(*threads));
    thread_data *threads_data = calloc(thr_count, sizeof(*threads_data));
    pthread_mutex_t *mutexes = calloc(acc_count, sizeof(*mutexes));
    for (uint32_t i = 0; i < acc_count; ++i) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
    double *accs = calloc(acc_count, sizeof(*accs));

    for (uint32_t i = 0; i < thr_count; ++i) {
        if (scanf("%" PRIu32 " %" PRIu32 " %lf %" PRIu32 " %lf",
                  &threads_data[i].iterations, &threads_data[i].acc_idx[0],
                  &threads_data[i].payment[0], &threads_data[i].acc_idx[1],
                  &threads_data[i].payment[1]) <= 0) {
            perror("scanf");
        }
        threads_data[i].accs = accs;
        threads_data[i].mutexes = mutexes;
    }
    for (int32_t i = 0; i < thr_count; ++i) {
        pthread_create(&threads[i], &attr, banking_staff, &threads_data[i]);
    }

    for (uint32_t i = 0; i < thr_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (uint32_t i = 0; i < acc_count; ++i) {
        printf("%.10g\n", accs[i]);
    }

    pthread_attr_destroy(&attr);
    free(threads_data);
    free(threads);
    return 0;
}