#include "checker2.h"

#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

// measurement tolerance in ms
#define TOLERANCE 20

/*
  argv[1] - test file
  argv[2] - output file
  argv[3] - correct file
  argv[4] - PID
 */
int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc != 6) {
        fatal_CF("wrong number of arguments %d", argc);
    }
    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        fatal_CF("cannot open test file '%s': %s", argv[1], strerror(errno));
    }
    long long timeout;
    if (fscanf(fin, "%lld", &timeout) != 1 || timeout < 0 || timeout > 1000000) {
        fatal_CF("invalid timeout value");
    }
    fclose(fin); fin = NULL;

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        fatal_CF("cannot open output file '%s': %s", argv[2], strerror(errno));
    }

    struct timeval cur;
    gettimeofday(&cur, NULL);
    long long curs = cur.tv_sec * 1000LL + (cur.tv_usec + 500) / 1000;

    long long expected = curs + timeout;
    printf("%lld %lld\n", expected / 1000, (expected % 1000) * 1000000);
    fclose(stdout);

    char buf[1024];
    int r = read(0, buf, sizeof(buf));
    if (r < 0) {
        fatal_CF("read failed: %s", strerror(errno));
    }
    if (r > 0) {
        checker_drain();
        fatal_PE("non-empty output to stdout");
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    long long ends = end.tv_sec * 1000LL + (end.tv_usec + 500) / 1000;
    long long actual = ends - curs;
    if (actual < 0) {
        fatal_CF("elapsed interval < 0, why?");
    }

    long long timediff = actual - timeout;
    if (timediff < 0) timediff = -timediff;
    if (timediff > TOLERANCE) {
        fprintf(fout, "%lld %lld %lld\n", timeout, actual, timediff);
        fatal_WA("time difference is too big");
    }
    return 0;
}
