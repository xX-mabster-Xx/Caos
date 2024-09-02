#include "checker2.h"
#include "testinfo.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
  argv[1] - test file
  argv[2] - output file
  argv[3] - correct file
  argv[4] - PID
  argv[5] - info file
 */
int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc != 6) {
        fatal_CF("wrong number of arguments: expected 6, but actual %d\n", argc);
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

    testinfo_t *ti = calloc(1, sizeof(*ti));
    int err = testinfo_parse(argv[5], ti, NULL);
    if (err < 0) {
        fatal_CF("Parsing of '%s' failed: %s", argv[4], testinfo_strerror(err));
    }

    if (ti->cmd.u != 1) {
        fatal_CF("Wrong number of arguments for program under testing");
    }

    int port = -1;
    if (checker_stoi(ti->cmd.v[0], 10, &port) < 0 || port <= 0 || (unsigned short) port != port) {
        fatal_CF("Invalid port number '%s'", ti->cmd.v[0]);
    }

    struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
    struct addrinfo *result = NULL;
    err = getaddrinfo("localhost", ti->cmd.v[0], &hints, &result);
    if (err) {
        fatal_CF("getaddrinfo() failed: %s", gai_strerror(err));
    }
    if (!result) {
        fatal_CF("result list is empty");
    }

    // let the program under testing start
    usleep(100000);

    int value = -1;
    while (1) {
        if (fscanf(fin, "%d", &value) != 1) {
            if (!value) break;
            value = 0;
        }

        int sfd = socket(PF_INET, SOCK_STREAM, 0);
        if (sfd < 0) {
            fatal_CF("socket() failed: %s", strerror(errno));
        }

        int val = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
        setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

        if (connect(sfd, result->ai_addr, result->ai_addrlen) < 0) {
            fprintf(stderr, "connect to the server failed: %s\n", strerror(errno));
            checker_kill(other_pid, SIGKILL);
            checker_drain();
            fatal_WA("cannot connect to the server");
        }

        value = htonl(value);
        err = write(sfd, &value, sizeof(value));
        if (err < 0) {
            fprintf(stderr, "write to server failed: %s\n", strerror(errno));
            checker_kill(other_pid, SIGKILL);
            checker_drain();
            fatal_WA("cannot connect to the server");
        }
        if (err != sizeof(value)) {
            fatal_CF("short write: %d", err);
        }
        // write()+close() doesn't sync
        // wait until server closes the connection
        // (without this, solutions with low backlog value can receive WT)
        shutdown(sfd, SHUT_WR);
        char foo;
        // If the solution doesn't close the socket, it will receive WT because of blocked read().
        // Not obvious, but better than WT caused by low backlog.
        err = read(sfd, &foo, sizeof(foo));
        if (err > 0) {
            checker_kill(other_pid, SIGKILL);
            checker_drain();
            fatal_WA("Garbage data received");
        } else if (err < 0) {
            // e.g. ECONNRESET if the connection was never accept()ed
            checker_kill(other_pid, SIGKILL);
            checker_drain();
            fatal_WA("read() failed: %s\n", strerror(errno));
        }
        if (close(sfd) < 0) {
            fprintf(stderr, "close failed: %s\n", strerror(errno));
            checker_kill(other_pid, SIGKILL);
            checker_drain();
            fatal_WA("close() failed");
        }
        if (!value) break;
    }

    char c;
    while ((c = getchar_unlocked()) != EOF) {
        putc_unlocked(c, fout);
    }
    return 0;
}
