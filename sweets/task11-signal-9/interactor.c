#include "checker2.h"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>

struct InputBuffer
{
    size_t size;
    size_t length;
    char *buf;
};

ssize_t read_available(int fd, struct InputBuffer *p)
{
    char buf[1024];

    p->length = 0;
    while (1) {
        int r = read(fd, buf, sizeof(buf));
        if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (p->length == 0) return -1;
            return p->length;
        }
        if (r < 0) {
            return -2;
        }
        if (!r) {
            return p->length;
        }
        if (!p->size) {
            size_t newsize = 64;
            while (newsize < r + 1) newsize *= 2;
            p->buf = malloc(newsize);
            p->size = newsize;
            memcpy(p->buf, buf, r);
            p->buf[r] = 0;
            p->length = r;
        } else {
            size_t newsize = p->size;
            while (newsize < p->length + r + 1) newsize *= 2;
            p->buf = realloc(p->buf, newsize);
            p->size = newsize;
            memcpy(p->buf + p->length, buf, r);
            p->length += r;
            p->buf[p->length] = 0;
        }
    }
}

void chop(char *str)
{
    if (!str) return;
    int len = strlen(str);
    while (len > 0 && isspace(str[len - 1])) --len;
    str[len] = 0;
}

/* TEST-INPUT TEST-OUTPUT CORRECT PID [INF-FILE] */
int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);
    fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

    int efd = epoll_create1(0);
    if (efd < 0) {
        fatal_CF("epoll_create1 failed");
    }
    struct epoll_event ev = { .events = EPOLLIN, .data = { .fd = 0 } };
    if (epoll_ctl(efd, EPOLL_CTL_ADD, 0, &ev) < 0) {
        fatal_CF("epoll_ctl failed");
    }

    struct InputBuffer ib = {};

    if (argc < 5) {
        fatal_CF("wrong number of arguments");
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        fatal_CF("cannot open input file '%s'", argv[1]);
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        fatal_CF("cannot open output file '%s'", argv[2]);
    }

    int other_pid = -1;
    if (checker_stoi(argv[4], 10, &other_pid) < 0 || other_pid <= 0) {
        fatal_CF("invalid process pid '%s'", argv[4]);
    }

    if (epoll_pwait(efd, &ev, 1, -1, NULL) < 0) {
        fatal_CF("epoll_pwait failed");
    }
    int r = read_available(0, &ib);
    if (r == -2) {
        fatal_CF("read error from pipe");
    }
    if (r == -1) {
        fatal_CF("data not available from pipe");
    }
    if (!r) {
        fatal_PE("unexpected EOF");
    }
    chop(ib.buf);

    int user_pid = -1;
    if (checker_stoi(ib.buf, 10, &user_pid) != 1) {
        checker_kill(other_pid, SIGKILL);
        checker_drain();
        fatal_PE("failed to parse PID");
    }
    if (user_pid != other_pid) {
        checker_kill(other_pid, SIGKILL);
        checker_drain();
        fatal_WA("user program reports invalid PID");
    }

    int val;
    while (fscanf(fin, "%d", &val) == 1) {
        if (val == 1) {
            checker_kill(other_pid, SIGUSR1);
            usleep(10000);
            if (epoll_pwait(efd, &ev, 1, -1, NULL) < 0) {
                fatal_CF("epoll_pwait failed");
            }
            r = read_available(0, &ib);
            if (r == -2) {
                fatal_CF("read error from pipe");
            }
            if (r == -1) {
                fatal_CF("no data available from pipe");
            }
            if (!r) {
                fatal_PE("unexpected EOF");
            }
            chop(ib.buf);

            //fprintf(stderr, ">>%s<<\n", ib.buf);
            int x1 = -1, x2 = -1;
            if (sscanf(ib.buf, "%d%d", &x1, &x2) != 2) {
                //checker_kill(other_pid, SIGKILL);
                checker_drain();
                fatal_PE("cannot parse program output");
            }
            fprintf(fout, "%d %d\n", x1, x2);
        } else if (val == 2) {
            checker_kill(other_pid, SIGUSR2);
            usleep(10000);
        } else if (val == 3) {
            checker_kill(other_pid, SIGTERM);
            checker_drain();
            break;
        } else {
            fatal_CF("invalid input value %d", val);
        }
    }
}
