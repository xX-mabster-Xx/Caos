#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

typedef struct thread_data {
    int my_number;
    int number_of_my_friends;
    int *fds;
} thread_data;

void *ping_pong(void *raw_data) {
    thread_data *data = raw_data;
    while (true) {
        uint64_t action;
        if (read(data->fds[data->my_number], &action, sizeof(action)) < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (action == 2) {
            return NULL;
        }

        int32_t input;

        if (scanf("%" PRId32, &input) <= 0) {
            for (int i = 0; i <= data->number_of_my_friends; ++i) {
                uint64_t out = 2;
                if (write(data->fds[i], &out, sizeof(out)) <= 0) {
                    perror("write1");
                    exit(EXIT_FAILURE);
                }
            }
            return NULL;
        }

        printf("%" PRId32 " %" PRId32 "\n", data->my_number, input);
        fflush(stdout);

        int32_t next =
            (input % data->number_of_my_friends + data->number_of_my_friends) %
            data->number_of_my_friends;

        uint64_t out = 1;
        if (write(data->fds[next], &out, sizeof(out)) <= 0) {
            perror("write2");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int count = atoi(argv[1]);

    pthread_t *threads = calloc(count, sizeof(*threads));
    thread_data *threads_data = calloc(count, sizeof(*threads_data));
    int *fds = calloc(count + 1, sizeof(*fds));

    for (int32_t i = 0; i < count; ++i) {
        threads_data[i].my_number = i;
        threads_data[i].number_of_my_friends = count;
        fds[i] = eventfd(0, 0);
        threads_data[i].fds = fds;
        pthread_create(&threads[i], NULL, ping_pong, &threads_data[i]);
    }
    fds[count] = eventfd(0, 0);
    uint64_t out = 1;
    if (write(fds[0], &out, sizeof(out)) <= 0) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    uint64_t action;
    if (read(fds[count], &action, sizeof(action)) < 0) {
        perror("read");
        return EXIT_FAILURE;
    }
    for (uint32_t i = 0; i < count; ++i) {
        pthread_join(threads[i], NULL);
    }

    for (int32_t i = 0; i < count; ++i) {
        close(fds[i]);
    }

    free(fds);
    free(threads_data);
    free(threads);
    return 0;
}