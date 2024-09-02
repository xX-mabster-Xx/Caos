#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

enum { NANO = 1000000000, NANO_TO_MICRO = 1000 };

void sigalrm_handler(int signo) {
    exit(0);
}

int timer_interval(struct timespec *ts, struct timeval *tv) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (ts->tv_sec < now.tv_sec ||
        (ts->tv_sec == now.tv_sec && ts->tv_nsec < now.tv_nsec)) {
        return -1;
    }

    if (ts->tv_nsec < now.tv_nsec) {
        tv->tv_usec = (NANO + ts->tv_nsec - now.tv_nsec) / NANO_TO_MICRO;
        tv->tv_sec = ts->tv_sec - now.tv_sec - 1;
        return 0;
    }
    tv->tv_usec = (ts->tv_nsec - now.tv_nsec) / NANO_TO_MICRO;
    tv->tv_sec = ts->tv_sec - now.tv_sec;
    return 0;
}

int main() {
    signal(SIGALRM, sigalrm_handler);

    time_t t;
    int ns;
    if (scanf("%ld %d", &t, &ns) < 0) {
        perror("scanf");
        return EXIT_FAILURE;
    }
    struct timespec ts = {
        .tv_sec = t,
        .tv_nsec = ns,
    };
    struct timeval tv;
    if (timer_interval(&ts, &tv) < 0) {
        return 0;
    }
    struct itimerval itv = {.it_interval = {0, 0}, .it_value = tv};
    setitimer(ITIMER_REAL, &itv, NULL);
    pause();
}