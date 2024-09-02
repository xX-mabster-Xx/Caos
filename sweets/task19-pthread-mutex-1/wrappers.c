#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>

static _Atomic int mutexes = 0;
static _Atomic int create_calls = 0;
static _Atomic int join_calls = 0;


int __real_pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void *), void *restrict arg);

int __wrap_pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void *), void *restrict arg) {
    create_calls += 1;
    return __real_pthread_create(thread, attr, start_routine, arg);
}

int __real_pthread_join(pthread_t thread, void **retval);

int __wrap_pthread_join(pthread_t thread, void **retval) {
    join_calls += 1;
    return __real_pthread_join(thread, retval);
}

int __real_main();

int __wrap_main() {
    int res = __real_main();
    if (create_calls != join_calls || create_calls != 3) {
        printf("Wrong number of threads\n");
        return 1;
    }
    if (mutexes == 0) {
        printf("Mutexes never used\n");
        return 1;
    }
    return res;
}

int __real_pthread_mutex_lock(pthread_mutex_t *restrict mutex);
int __wrap_pthread_mutex_lock(pthread_mutex_t *restrict mutex) {
    mutexes += 1;
    return __real_pthread_mutex_lock(mutex);
}
