#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

enum { DEFAULT = 0, BROKEN = 1, MISSING = 2 };

int define_type(char *path) {
    struct stat file_info;
    lstat(path, &file_info);
    if (S_ISLNK(file_info.st_mode) && access(path, F_OK) == -1) {
        return BROKEN;
    }
    if (access(path, F_OK) == -1) {
        return MISSING;
    }
    return DEFAULT;
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        int type = define_type(argv[i]);
        if (type == DEFAULT) {
            printf("%s\n", argv[i]);
        } else if (type == BROKEN) {
            printf("%s (broken symlink)\n", argv[i]);
        } else {
            printf("%s (missing)\n", argv[i]);
        }
    }
}
