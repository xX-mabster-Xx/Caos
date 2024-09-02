#define _XOPEN_SOURCE
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
    FILE *file = fopen(argv[1], "r");

    char buffer[1024];
    struct tm tm;
    time_t prev_t = 0;

    int first = 1;

    while (fgets(buffer, sizeof(buffer) - 1, file) != NULL) {
        if (strptime(buffer, "%Y/%m/%d %H:%M:%S", &tm) != NULL) {
            tm.tm_isdst = -1;
            time_t t = mktime(&tm);
            if (first) {
                first = 0;
            } else {
                printf("%ld\n", t - prev_t);
            }
            prev_t = t;
        }
    }

    fclose(file);
}