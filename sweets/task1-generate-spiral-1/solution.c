#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum { FLAGS = 07777 };

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, FLAGS);
    int rows = atoi(argv[2]);
    int cols = atoi(argv[3]);
    ftruncate(fd, sizeof(int) * rows * cols);
    int *array = mmap(0, sizeof(int) * cols * rows, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);
    memset(array, 0, sizeof(int) * rows * cols);
    if (rows * cols > 0) {
        array[0] = 1;
    }
    int shifts[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int n = 1;
    int x = 0, y = 0;
    for (int i = 0; n < rows * cols; ++i) {
        int shift_i = i % 4;
        while (
            x + shifts[shift_i][0] < rows && y + shifts[shift_i][1] < cols &&
            array[(x + shifts[shift_i][0]) * cols + y + shifts[shift_i][1]] ==
                0) {
            x += shifts[shift_i][0];
            y += shifts[shift_i][1];
            array[x * cols + y] = ++n;
        }
    }
}
