#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

volatile sig_atomic_t sigusr2_cnt = 0;
volatile sig_atomic_t sigusr1_flag = 0;
volatile sig_atomic_t sigterm_flag = 0;

void sigusr2_handler(int) {
    sigusr2_cnt++;
}

void sigusr1_handler(int) {
    sigusr1_flag = 1;
}

void sigterm_handler(int) {
    sigterm_flag = 1;
}

int main() {
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);
    signal(SIGTERM, sigterm_handler);

    sigset_t blocked_mask, open_mask, old_mask;
    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, SIGUSR1);
    sigaddset(&blocked_mask, SIGUSR2);
    sigaddset(&blocked_mask, SIGTERM);
    sigemptyset(&open_mask);

    if (sigprocmask(SIG_BLOCK, &blocked_mask, &old_mask) < 0) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    printf("%d\n", getpid());
    fflush(stdout);

    int sigusr1_cnt = 0;

    while (1) {
        sigsuspend(&open_mask);
        if (sigprocmask(SIG_BLOCK, &blocked_mask, NULL) < 0) {
            perror("sigprocmask");
            return EXIT_FAILURE;
        }
        if (sigterm_flag) {
            if (sigprocmask(SIG_SETMASK, &old_mask, NULL) < 0) {
                perror("sigprocmask");
                return EXIT_FAILURE;
            }
            exit(0);
        }
        if (sigusr1_flag) {
            sigusr1_flag = 0;
            printf("%d %d\n", sigusr1_cnt, sigusr2_cnt);
            fflush(stdout);
            ++sigusr1_cnt;
        }
    }
}