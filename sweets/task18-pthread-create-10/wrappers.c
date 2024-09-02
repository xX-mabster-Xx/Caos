#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

enum { N = 10 };

static _Atomic pid_t main_tid = 0;
static int create_calls = 0;
static int join_calls = 0;

struct task {
    void *(*func)(void *);
    void *arg;
    useconds_t delay;
};

static void* run_with_delay(void *arg) {
    struct task *tsk = arg;
    struct task tsk_local;
    memcpy(&tsk_local, tsk, sizeof(tsk_local));
    free(tsk);
    usleep(tsk_local.delay);
    return tsk_local.func(tsk_local.arg);
}

int __real_pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void *), void *restrict arg);
int __wrap_pthread_create(pthread_t *restrict thread, const pthread_attr_t *restrict attr,
                          void *(*start_routine)(void *), void *restrict arg) {
    pid_t main_tid_loc = main_tid;
    pid_t self_tid = syscall(SYS_gettid);
    if (main_tid_loc == 0) {
        // 1st thread_create() call is always from main thread
        main_tid = self_tid;
        main_tid_loc = self_tid;
    }
    if (self_tid != main_tid_loc) {
        printf("All threads must be created by main thread\n");  // print to stdout to get PE
        fflush(stdout);
        _exit(0);
    }

    int thread_num = create_calls++;
    if (thread_num == 0 || thread_num == N - 1) {
        // Ban busy wait: delay 1st and last function calls by 500ms
        // (N - 1 is for solutions with reverse loop)
        struct task *tsk = calloc(1, sizeof(*tsk));
        tsk->arg = arg;
        tsk->func = start_routine;
        tsk->delay = 500000;
        start_routine = run_with_delay;
        arg = tsk;
    }
    return __real_pthread_create(thread, attr, start_routine, arg);
}

int __real_pthread_join(pthread_t thread, void **retval);
int __wrap_pthread_join(pthread_t thread, void **retval) {
    pid_t main_tid_loc = main_tid;
    if (main_tid_loc == 0) {
        fprintf(stderr, "pthread_join is called before pthread_create");
        _exit(1);
    } else if (syscall(SYS_gettid) == main_tid_loc) {
        if (++join_calls > 1) {
            printf("Main thread can call pthread_join at most once\n");  // print to stdout to get PE
            fflush(stdout);
            _exit(0);
        }
    }
    return __real_pthread_join(thread, retval);
}

int __real_main();
int __wrap_main() {
    int res = __real_main();
    if (main_tid == 0) {
        printf("No threads created\n");
        return 1;
    }
    if (create_calls != 10) {
        printf("Not enaught threads created\n");
        return 1;
    }
    return res;
}
