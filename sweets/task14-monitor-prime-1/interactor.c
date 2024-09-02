#include "checker2.h"

#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

enum { MAX_ENTRY = 32 };
enum { TOLERANCE = 20 };

struct Entry
{
    long long value;
    int utime;
};

int isprime(unsigned value)
{
    if (!(value & 1)) return 0;
    unsigned m = (unsigned) sqrt(value) + 1;
    for (unsigned d = 3; d <= m; d += 2) {
        if (!(value % d)) return 0;
    }
    return 1;
}

int get_proc_utime(int pid, int clock_ticks)
{
    char path[PATH_MAX];
    FILE *f = NULL;
    char buf[8192];
    int blen;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    if (!(f = fopen(path, "r"))) {
        return -2;
    }
    if (!fgets(buf, sizeof(buf), f)) goto fail;
    blen = strlen(buf);
    if (blen + 1 == sizeof(buf)) goto fail;
    fclose(f); f = NULL;

    char *p = strrchr(buf, ')');
    if (!p) goto fail;
    ++p;
    if (*p != ' ') goto fail;
    // skip state
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip ppid
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip pgrp
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip session
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip tty_nr
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip tpgid
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip flags
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip minflt
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip cminflt
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip majflt
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;
    // skip cmajflt
    ++p;
    if (!(p = strchr(p, ' '))) goto fail;

    errno = 0;
    long value = strtol(p, NULL, 10);
    if (errno) goto fail;
    if ((int) value != value) goto fail;
    if (value < 0) goto fail;
    value = value * 1000 / clock_ticks;
    return value;

fail:
    fprintf(stderr, "failed line: '%s'\n", buf);
    if (f) fclose(f);
    return -2;
}

/*
  argv[1] - test file
  argv[2] - output file
  argv[3] - correct file
  argv[4] - PID
 */
int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    if (argc < 5) {
        fatal_CF("wrong number of arguments");
    }

    int pid;
    if (checker_stoi(argv[4], 10, &pid) < 0 || pid <= 1) {
        fatal_CF("invalid PID");
    }

    int clock_ticks = sysconf(_SC_CLK_TCK);
    if (clock_ticks <= 0) {
        fatal_CF("cannot obtain clock ticks");
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        fatal_CF("cannot open test file '%s': %s", argv[1], strerror(errno));
    }
    unsigned low, high;
    if (fscanf(fin, "%u%u", &low, &high) != 2) {
        fatal_CF("cannot read values low, high");
    }
    if (low < 1 || low > high || high >= UINT_MAX - 10) {
        fatal_CF("invalid range low = %u, high = %u", low, high);
    }
    fclose(fin); fin = NULL;

    printf("%u\n%u\n", low, high);
    fclose(stdout);

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        fatal_CF("cannot open output file '%s': %s", argv[2], strerror(errno));
    }

    struct Entry entries[MAX_ENTRY];
    int entry_count = 0;

    while (1) {
        char buf[1024];
        int r = read(0, buf, sizeof(buf));
        if (r < 0) {
            fatal_CF("read error: %s", strerror(errno));
        }
        if (!r) break;
        if (entry_count == MAX_ENTRY) {
            checker_drain();
            fatal_PE("too many numbers from program");
        }
        int utime = get_proc_utime(pid, clock_ticks);
        if (utime == -2) {
            checker_drain();
            fatal_PE("process terminated prematurely");
        }
        if (utime < 0) {
            fatal_CF("cannot parse process utime");
        }
        entries[entry_count].utime = utime;

        if (r >= (int) sizeof(buf) - 10) {
            checker_drain();
            fatal_PE("program output is too big");
        }
        if (buf[r - 1] != '\n') {
            checker_drain();
            fatal_PE("program output line does not end with \\n");
        }
        if (strchr(buf, '\n') != &buf[r - 1]) {
            checker_drain();
            fatal_PE("program outputs several lines at once");
        }
        while (r > 0 && isspace(buf[r - 1])) --r;
        buf[r] = 0;
        if (!r) {
            checker_drain();
            fatal_PE("program outputs empty line");
        }

        errno = 0;
        char *eptr = NULL;
        long long value = strtoll(buf, &eptr, 10);
        if (errno || *eptr) {
            checker_drain();
            fatal_PE("program outputs not a number");
        }
        entries[entry_count].value = value;

        fprintf(stderr, "%d %lld\n", entries[entry_count].utime, entries[entry_count].value);
        ++entry_count;
    }

    if (entry_count > 8) {
        fatal_PE("output contains %d lines instead of 8", entry_count);
    }

    for (int i = 0; i < entry_count; ++i) {
        long long value = entries[i].value;
        int utime = entries[i].utime;

        if (value < 0) {
            fatal_PE("negative value in output");
        }
        if (value == 0 || value == -1) {
            fatal_WA("0 or -1 should never appear in output");
        }
        if (value < low || value >= high) {
            fatal_WA("value %lld out of range", value);
        }
        if (!isprime(value)) {
            fatal_WA("value %lld is not prime", value);
        }
        if (i > 0 && entries[i - 1].value > value) {
            fatal_WA("value %lld is less than previous", value);
        }
        int timediff = utime - (i + 1) * 100;
        if (timediff < 0) timediff = -timediff;
        if (timediff > TOLERANCE) {
            fatal_WA("user time difference is too big for %d", utime);
        }
    }
}
