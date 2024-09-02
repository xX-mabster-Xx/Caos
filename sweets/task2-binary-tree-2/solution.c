#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Node {
    int32_t key;
    int32_t left_idx;
    int32_t right_idx;
} Node;

void tree_printer(Node *base, size_t shift) {
    Node *node = base + shift;
    if (node->right_idx != 0) {
        tree_printer(base, node->right_idx);
    }
    printf("%" PRId32 "\n", node->key);
    if (node->left_idx != 0) {
        tree_printer(base, node->left_idx);
    }
}

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        return EXIT_FAILURE;
    }
    off_t filesize = lseek(fd, 0, SEEK_END);
    Node *data = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    tree_printer(data, 0);
    close(fd);
}