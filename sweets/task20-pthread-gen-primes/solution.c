#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

pthread_cond_t found_new_prime = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

enum { THREADS_COUNT = 10 };

uint64_t prime = 0;

typedef struct thread_data {
    uint64_t base;
} thread_data;

void *find_primes(void *raw_data) {
    thread_data *data = raw_data;

    for (uint64_t num = data->base;; ++num) {
        uint64_t i = 2;
        for (; i * i <= num && num % i != 0; ++i) {
        }
        if (i * i > num) {
            pthread_mutex_lock(&mutex);
            prime = num;
            pthread_cond_signal(&found_new_prime);
            pthread_mutex_unlock(&mutex);
        }
    }

    return NULL;
}

int main() {
    uint64_t base = 0;
    uint32_t count = 0;

    if (scanf("%" PRIu64 " %" PRIu32, &base, &count) <= 0) {
        perror("scanf");
        return EXIT_FAILURE;
    }

    thread_data data = {
        .base = base,
    };
    pthread_t thread;
    uint64_t last_prime = 0;
    pthread_create(&thread, NULL, find_primes, &data);
    for (int i = 0; i < count; ++i) {
        pthread_mutex_lock(&mutex);
        while (last_prime == prime) {
            pthread_cond_wait(&found_new_prime, &mutex);
        }
        last_prime = prime;
        pthread_mutex_unlock(&mutex);
        printf("%" PRIu64 "\n", last_prime);
    }
    return 0;
}