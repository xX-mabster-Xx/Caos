#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

// TODO use tgz instead?

int __real_main(int argc, char **argv, char **envp);
int __wrap_main(int argc, char **argv, char **envp) {
    assert(argc == 4);
    char *prefill_str = getenv("PREFILL_SIZE");
    if (prefill_str) {
        long prefill_size = strtol(prefill_str, NULL, 10);
        int fd = open(argv[1], O_RDWR | O_CREAT, 0644);
        assert(fd >= 0);
        assert(ftruncate(fd, prefill_size) == 0);
        close(fd);
    }
    return __real_main(argc, argv, envp);
}
