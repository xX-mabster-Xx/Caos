#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

enum { THREADS_COUNT = 10 };

typedef struct thread_data {
    int number;
    pthread_t wait;
} thread_data;

void *print_my_number(void *raw_data) {
    thread_data *data = raw_data;
    if (data->number != 0) {
        pthread_join(data->wait, NULL);
    }
    printf("%d\n", data->number);
    return NULL;
}

int main() {
    pthread_t thread;
    thread_data all_data[10];
    for (int i = 0; i < THREADS_COUNT; ++i) {
        all_data[i].number = i;
        all_data[i].wait = thread;
        pthread_create(&thread, NULL, print_my_number, &all_data[i]);
    }
    pthread_join(thread, NULL);
}