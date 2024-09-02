#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>

static _Atomic int waits = 0;

int __real_pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
int __wrap_pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex) {
    ++waits;
    // In Linux, spurious wakeups cannot be caused by the OS.
    // But POSIX allows such wakeups, so we can simulate them.
    static _Atomic int cached_spurious_wakeups = -1;
    int spurious_wakeups = atomic_load(&cached_spurious_wakeups);
    if (spurious_wakeups == -1) {
        // This part can be executed more than once if 2+ threads call pthread_cond_wait concurrently,
        // but it doesn't cause data races.
        if (getenv("EJ_SPURIOUS_WAKEUPS")) {
            spurious_wakeups = 1;
        } else {
            spurious_wakeups = 0;
        }
        atomic_store(&cached_spurious_wakeups, spurious_wakeups);
    }
    if (spurious_wakeups) {
        static _Atomic size_t i = 0;
        if (atomic_fetch_add(&i, 1) % 2 == 1) {
            return 0;
        }
    }
    return __real_pthread_cond_wait(cond, mutex);
}

int __real_main();
int __wrap_main() {
    int res = __real_main();
    if (waits == 0) {
        printf("Condwar not used\n");
        return 1;
    }
    return res;
}
