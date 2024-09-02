#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>


// Plain int is enough for forking main.
// Atomic is used to prevent data races if the implementation is changed.
#define COND_SLEEP(varname)                                                    \
    static _Atomic int varname = -1;                                           \
    if (varname == -1) {                                                       \
        varname = getenv(#varname) != NULL;                                    \
    }                                                                          \
    if (varname) {                                                             \
        usleep(100000);                                                        \
    }

void futex_wait(_Atomic int *addr, int val) {
    COND_SLEEP(EJ_SLOW_WAIT);
    // atomically: block on addr if (*addr == val)
    syscall(SYS_futex, addr, FUTEX_WAIT, val, NULL, NULL, 0);
}

void futex_wake(_Atomic int *addr, int num) {
    COND_SLEEP(EJ_SLOW_WAKE);
    // wake up to num threads blocked on addr
    syscall(SYS_futex, addr, FUTEX_WAKE, num, NULL, NULL, 0);
}


/// I'm a bad guy
#include "solution.c"



struct state {
    caos_mutex_t mutex;
    int value;
};

enum {
    PROCESSES = 10,
    LOCKS = 100000,
};

void slow_thread(struct state *state) {
    caos_mutex_lock(&state->mutex);
    nanosleep(&(struct timespec){.tv_nsec = 50000000}, NULL);
    state->value++;
    caos_mutex_unlock(&state->mutex);
}

void racy_thread(struct state *state, int i) {
    caos_mutex_lock(&state->mutex);
    int value = state->value;
    asm volatile("nop" ::: "memory");
    nanosleep(&(struct timespec){.tv_nsec = 10000000 * (i + 1)}, NULL);
    asm volatile("nop" ::: "memory");
    state->value = value + 1;
    caos_mutex_unlock(&state->mutex);
}

void single_thread(struct state *state) {
    for (int j = 0; j < LOCKS; ++j) {
        caos_mutex_lock(&state->mutex);
        state->value++;
        caos_mutex_unlock(&state->mutex);
    }
    printf("%d\n", state->value);
}

int main() {
    struct state *state = mmap(NULL, sizeof(*state), PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    caos_mutex_init(&state->mutex);
    if (getenv("EJ_SINGLE_THREAD")) {
        single_thread(state);
        return 0;
    }
    for (int i = 0; i < PROCESSES; ++i) {
        if (!fork()) {
            if (getenv("EJ_SLOW_THREADS")) {
                slow_thread(state);
            } else if (getenv("EJ_RACY_THREADS")) {
                racy_thread(state, i);
            } else {
                for (int j = 0; j < LOCKS; ++j) {
                    caos_mutex_lock(&state->mutex);
                    state->value++;
                    caos_mutex_unlock(&state->mutex);
                }
            }
            return 0;
        }
    }
    while (wait(NULL) > 0) {
        ;
    }
    printf("%d\n", state->value);
}
