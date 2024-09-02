#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

void futex_wait(_Atomic int *addr, int val);
void futex_wake(_Atomic int *addr, int num);

typedef struct mutex {
    _Atomic int lock;
    _Atomic int waiters;
} caos_mutex_t;

void caos_mutex_init(caos_mutex_t *m) {
    m->lock = 0;
    m->waiters = 0;
}

void caos_mutex_lock(caos_mutex_t *m) {
    int expected = 0;
    while (!atomic_compare_exchange_strong(&m->lock, &expected, 1)) {
        m->waiters++;
        futex_wait(&m->lock, 1);
        m->waiters--;
        expected = 0;
    }
}

void caos_mutex_unlock(caos_mutex_t *m) {
    m->lock = 0;
    if (m->waiters > 0) {
        futex_wake(&m->lock, 1);
    }
}
