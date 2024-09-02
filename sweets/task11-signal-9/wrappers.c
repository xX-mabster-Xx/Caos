#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

int __real_sigsuspend(const sigset_t *mask);
int __real_pause(void);
int __real_sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oldset);
int __real_sigaction(int signum, const struct sigaction *restrict act, struct sigaction *restrict oldact);
sighandler_t __real_signal(int signum, sighandler_t handler);

void __real_exit(int status);
void __real__exit(int status);
void __real__Exit(int status);

int __real_main(int argc, char **argv, char **envp);

// ----- sigprocmask/sigsuspend delay (check if signals are properly blocked) -----

static int spm_first_call = 1;
static int ss_first_call = 1;
static int pause_first_call = 1;
static int need_delay = -1;

static void delay_if_needed(int *first_call) {
    if (need_delay == -1) {
        need_delay = (getenv("DELAY_SIGSUSPEND") != NULL);
    }
    if (need_delay && *first_call) {
        usleep(20000);
        *first_call = 0;
    }
}

int __wrap_sigprocmask(int how, const sigset_t *restrict set, sigset_t *restrict oldset) {
    // print pid -> set mask: signal may be missed
    delay_if_needed(&spm_first_call);
    return __real_sigprocmask(how, set, oldset);
}

int __wrap_sigsuspend(const sigset_t *mask) {
    // if signals are not blocked, usleep will be interrupted and real sigsuspend will miss the signal
    delay_if_needed(&ss_first_call);
    return __real_sigsuspend(mask);
}

int __wrap_pause(void) {
    // same as sigsuspend
    delay_if_needed(&pause_first_call);
    return __real_pause();
}

// ----- Use alt stack for SIGTERM handler (ban exit() in handlers) -----

static stack_t sig_stack;

int __wrap_main(int argc, char **argv, char **envp) {
    int stack_size = MINSIGSTKSZ + 16 * 1024;  // NOLINT

    sig_stack.ss_sp = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (sig_stack.ss_sp == MAP_FAILED) {
        abort();
    }
    sig_stack.ss_size = stack_size;
    if(sigaltstack(&sig_stack, NULL) < 0) {
        abort();
    }

    // Block signals (masks are preserved across exec*, so initial mask may contain any signals and shouldn't be used for sigsuspend)
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGTERM);
    sigaddset(&sigs, SIGUSR1);
    sigaddset(&sigs, SIGUSR2);
    __real_sigprocmask(SIG_BLOCK, &sigs, NULL);

    return __real_main(argc, argv, envp);
}

int __wrap_sigaction(int signum, const struct sigaction *restrict act, struct sigaction *restrict oldact) {
    if (signum != SIGTERM) {
        return __real_sigaction(signum, act, oldact);
    }
    struct sigaction newact = *act;
    newact.sa_flags |= SA_ONSTACK;
    return __real_sigaction(signum, &newact, oldact);
}

sighandler_t __wrap_signal(int signum, sighandler_t handler) {
    if (signum != SIGTERM) {
        return __real_signal(signum, handler);
    }
    // Use BSD semantics.
    // Real signal() implementation may differ, but this should be ok for SIGTERM.
    struct sigaction oldact;
    struct sigaction newact = { .sa_handler = handler, .sa_flags = SA_RESTART | SA_ONSTACK };
    if (__real_sigaction(signum, &newact, &oldact) < 0) {
        return SIG_ERR;
    }
    return oldact.sa_handler;
}

static void assert_not_handler() {
    int foo = 0;
    if ((uintptr_t)&foo - (uintptr_t)sig_stack.ss_sp < sig_stack.ss_size) {
        const char *err = "_exit was called from a signal handler\n";
        if (write(STDERR_FILENO, err, strlen(err))) {}
        abort();
    }
}

void __wrap_exit(int status) {
    assert_not_handler();
    __real_exit(status);
}

void __wrap__exit(int status) {
    assert_not_handler();
    __real__exit(status);
}

void __wrap__Exit(int status) {
    assert_not_handler();
    __real__Exit(status);
}
