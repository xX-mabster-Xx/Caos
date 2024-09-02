#include <sys/stat.h>
#include <valgrind/memcheck.h>

int __real_stat(const char *restrict pathname, struct stat *restrict statbuf);
int __real_fstat(int fd, struct stat *statbuf);
int __real_lstat(const char *restrict pathname, struct stat *restrict statbuf);
int __real_fstatat(int dirfd, const char *restrict pathname,
        struct stat *restrict statbuf, int flags);
int __real___xstat(int ver, const char *pathname, struct stat *stat_buf);
int __real___fxstat(int ver, int fd, struct stat *stat_buf);
int __real___lxstat(int ver, const char *pathname, struct stat *stat_buf);
int __real___fxstatat(int ver, int dirfd, const char *restrict pathname,
        struct stat *restrict statbuf, int flags);

int __wrap_stat(const char *restrict pathname, struct stat *restrict statbuf) {
    int res = __real_stat(pathname, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap_fstat(int fd, struct stat *statbuf) {
    int res = __real_fstat(fd, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap_lstat(const char *restrict pathname, struct stat *restrict statbuf) {
    int res = __real_lstat(pathname, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap_fstatat(int dirfd, const char *restrict pathname,
        struct stat *restrict statbuf, int flags) {
    int res = __real_fstatat(dirfd, pathname, statbuf, flags);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap___xstat(int ver, const char *pathname, struct stat *statbuf) {
    int res = __real___xstat(ver, pathname, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap___fxstat(int ver, int fd, struct stat *statbuf) {
    int res = __real___fxstat(ver, fd, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap___lxstat(int ver, const char *pathname, struct stat *statbuf) {
    int res = __real___lxstat(ver, pathname, statbuf);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}

int __wrap___fxstatat(int ver, int dirfd, const char *restrict pathname,
        struct stat *restrict statbuf, int flags) {
    int res = __real___fxstatat(ver, dirfd, pathname, statbuf, flags);
    if (res < 0) {
        VALGRIND_MAKE_MEM_UNDEFINED(statbuf, sizeof(*statbuf));
    }
    return res;
}
