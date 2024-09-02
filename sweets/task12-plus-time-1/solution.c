#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

enum { SEC_IN_DAY = 24 * 60 * 60 };

int main() {
    time_t days;
    while (scanf("%ld", &days) > 0) {
        time_t t = time(NULL);
        time_t delta;
        if (__builtin_mul_overflow(days, SEC_IN_DAY, &delta)) {
            printf("OVERFLOW\n");
            continue;
        }
        time_t result;
        if (__builtin_add_overflow(t, days * SEC_IN_DAY, &result)) {
            printf("OVERFLOW\n");
            continue;
        }
        struct tm lt;
        localtime_r(&result, &lt);
        char buf[1024] = {0};
        strftime(buf, sizeof(buf) - 1, "%Y-%m-%d", &lt);
        printf("%s\n", buf);
    }
}