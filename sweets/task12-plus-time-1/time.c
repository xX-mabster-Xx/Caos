#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

time_t __real_time(time_t *pt);
time_t __wrap_time(time_t *pt)
{
    const char *tstr = getenv("EJ_TIME");
    if (!tstr) {
        return __real_time(pt);
    }
    int y, m, d, h, mm, s;
    sscanf(tstr, "%d-%d-%d %d:%d:%d", &y, &m, &d, &h, &mm, &s);
    struct tm tt;
    memset(&tt, 0, sizeof(tt));
    tt.tm_year = y - 1900;
    tt.tm_mon = m - 1;
    tt.tm_mday = m;
    tt.tm_hour = h;
    tt.tm_min = mm;
    tt.tm_sec = s;
    tt.tm_isdst = -1;
    time_t nt = mktime(&tt);
    if (nt == (time_t) -1) {
        return __real_time(pt);
    }
    if (pt) *pt = nt;
    return nt;
}
