#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum { NANO = 1000000000, ALMOSTNANO = 100000000 };

int passed(long long dec_s, struct timespec begin, unsigned prime) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if ((now.tv_sec - begin.tv_sec) * NANO + (long long)now.tv_nsec -
            (long long)begin.tv_nsec >
        dec_s * ALMOSTNANO) {
        printf("%u\n", prime);
        fflush(stdout);
        if (dec_s == 8) {
            exit(0);
        }
        return 1;
    }
    return 0;
}

int main() {
    struct timespec begin;
    unsigned low, high;
    if (scanf("%u %u", &low, &high) <= 0) {
        perror("scanf");
        return EXIT_FAILURE;
    }
    clock_gettime(CLOCK_MONOTONIC, &begin);
    unsigned last_prime = 0;
    long long timer = 1;
    for (unsigned i = low; i < high; ++i) {
        unsigned j = 2;
        while (j * j <= i && i % j != 0) {
            ++j;
            timer += passed(timer, begin, last_prime);
        }
        if (j * j > i) {
            last_prime = i;
        }
        timer += passed(timer, begin, last_prime);
    }
    printf("%d\n", -1);
    return 0;
}