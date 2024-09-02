#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char *concat_path(const char *a, const char *b) {
    size_t len_of_a = strlen(a);
    char *ptr = calloc(len_of_a + strlen(b) + 1, sizeof(char));
    strcpy(ptr, a);
    ptr[len_of_a] = '/';
    strcpy(ptr + len_of_a + 1, b);
    return ptr;
}

void traverse(const char *dir1, const char *dir2, long long maxsize,
              int depth) {
    if (depth > 4) {
        return;
    }
    DIR *dir = opendir(dir1);
    for (struct dirent *file = readdir(dir); file != NULL;
         file = readdir(dir)) {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            continue;
        }
        const char *real_path = concat_path(dir1, file->d_name);
        const char *show_path = concat_path(dir2, file->d_name);

        if (file->d_type == DT_DIR && access(real_path, R_OK | X_OK) == 0) {
            traverse(real_path, show_path, maxsize, depth + 1);
        } else if (file->d_type == DT_REG && access(real_path, R_OK) == 0) {
            struct stat file_info;
            stat(real_path, &file_info);
            if (file_info.st_size <= maxsize) {
                printf("%s\n", show_path + 1);
            }
        }

        free((void *)show_path);
        free((void *)real_path);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *dir = argv[1];
    long long maxsize = strtoll(argv[2], NULL, 10);
    traverse(dir, "", maxsize, 1);
}
