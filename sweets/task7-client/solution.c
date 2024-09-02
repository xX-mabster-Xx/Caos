#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_connection(char *node, char *service) {
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
        .ai_family = AF_UNSPEC,  // можно и AF_INET, и AF_INET6
        .ai_socktype = SOCK_STREAM,  // но мы хотим поток (соединение)
    };
    if ((gai_err = getaddrinfo(node, service, &hint, &res))) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(gai_err));
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            perror("socket");
            continue;
        }
        if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("connect");
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

int main(int argc, char *argv[]) {
    struct sigaction sa = {
        .sa_handler = SIG_IGN,
        .sa_flags = 0,
    };
    sigaction(SIGPIPE, &sa, NULL);
    int sock = create_connection(argv[1], argv[2]);
    if (sock < 0) {
        return 1;
    }
    FILE *fin = fdopen(sock, "r");
    FILE *fout = fdopen(dup(sock), "w");
    if (fprintf(fout, "%s\n", argv[3]) < 0) {
        return 0;
    }
    if (fflush(fout) < 0) {
        return 0;
    }
    unsigned long long k;
    if (fscanf(fin, "%llu", &k) <= 0) {
        return 0;
    }
    for (unsigned long long i = 0; i <= k; ++i) {
        if (fprintf(fout, "%llu\n", i) < 0) {
            return 0;
        }
    }
    if (fflush(fout) != 0) {
        return 0;
    }
    unsigned long long answer;
    if (fscanf(fin, "%llu", &answer) <= 0) {
        return 0;
    }
    printf("%llu\n", answer);
    fclose(fin);
    fclose(fout);
}