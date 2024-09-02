#include <signal.h>
#include <unistd.h>

void terms_sayer(int signo) {
    write(STDOUT_FILENO, "We communicate on my terms.\n", 28);
}

int main() {
    signal(SIGTERM, terms_sayer);
    char buf[1024];
    ssize_t bytes;
    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, bytes);
    }
}
