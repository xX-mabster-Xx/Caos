#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

static _Atomic int mutexes = 0;
static _Atomic int pipes = 0;
static _Atomic int evfds = 0;
static _Atomic int create_calls = 0;
static _Atomic int join_calls = 0;
static _Atomic pid_t prev_id = 0;
//static _Atomic pid_t prev_id_printf = 0;


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

int __real_main(int argc, char *argv[]);
int __wrap_main(int argc, char *argv[]) {
    int threads_num = atoi(argv[1]);
    int res = __real_main(argc, argv);
    if (create_calls != join_calls || create_calls != threads_num) {
        fprintf(stderr, "Wrong number of threads\n");
        return 1;
    }
    if (mutexes != 0) {
        fprintf(stderr, "Mutexes used\n");
        return 1;
    }
    if (evfds == 0 && pipes == 0) {
        fprintf(stderr, "eventfd and pipe not used\n");
        return 1;
    }
    return res;
}

int __real_pthread_mutex_lock(pthread_mutex_t *restrict mutex);
int __wrap_pthread_mutex_lock(pthread_mutex_t *restrict mutex) {
    mutexes += 1;
    return __real_pthread_mutex_lock(mutex);
}

int __real_pipe(int pipefd[2]);
int __wrap_pipe(int pipefd[2]) {
    pipes += 1;
    return __real_pipe(pipefd);
}

int __real_eventfd(unsigned int initval, int flags);
int __wrap_eventfd(unsigned int initval, int flags) {
    evfds += 1;
    return __real_eventfd(initval, flags);
}

ssize_t __real_write(int fildes, const void *buf, size_t nbyte);
ssize_t __wrap_write(int fildes, const void *buf, size_t nbyte) {
    if (fildes == stdout->_fileno) {
        pid_t self_tid = syscall(SYS_gettid);
        if (prev_id == self_tid) {
            fprintf(stderr, "Unexpected thread output\n");
            exit(1);
        }
        prev_id = self_tid;
    }
    return __real_write(fildes, buf, nbyte);
}
//int __real_printf(const char *format, va_list argp);
//int __wrap_printf(const char *format, va_list argp) {
//    pid_t self_tid = syscall(SYS_gettid);
//    if (prev_id_printf == self_tid) {
//        fprintf(stderr, "Unexpected thread output\n");
//        exit(1);
//    }
//    prev_id_printf = self_tid;
//    return __real_printf(format, argp);
//}
//
//int __real_fprintf(FILE *stream, const char *format, va_list argp);
//int __wrap_fprintf(FILE *stream, const char *format, va_list argp) {
//    if (stream == stdout) {
//        pid_t self_tid = syscall(SYS_gettid);
//        if (prev_id_printf == self_tid) {
//            fprintf(stderr, "Unexpected thread output\n");
//            exit(1);
//        }
//        prev_id_printf = self_tid;
//    }
//    return __real_fprintf(stream, format, argp);
//}
