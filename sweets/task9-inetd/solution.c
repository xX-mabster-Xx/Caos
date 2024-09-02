#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int create_listener(char *service) {
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hint = {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM,
        .ai_flags =
            AI_PASSIVE,  // get addresses suitable for a server to bind to
    };
    if ((gai_err = getaddrinfo(NULL, service, &hint, &res))) {
        fprintf(stderr, "gai error: %s\n", gai_strerror(gai_err));
        return -1;
    }
    int sock = -1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        // create socket of the suitable family (AF_INET or AF_INET6)
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            perror("socket");
            continue;
        }

        // make port immediately reusable after we release it
        int one = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
            perror("setsockopt");
            goto err;
        }

        // try to bind and listen
        if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("bind");
            goto err;
        }
        if (listen(sock, SOMAXCONN) < 0) {
            perror("listen");
            goto err;
        }
        break;
err:
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    return sock;
}

void sigterm_handler(int) {
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, sigterm_handler);

    int sock = create_listener(argv[1]);
    if (sock < 0) {
        return 1;
    }
    while (1) {
        int connection = accept(sock, NULL, NULL);
        if (fork() == 0) {
            close(sock);
            dup2(connection, STDIN_FILENO);
            dup2(connection, STDOUT_FILENO);
            close(connection);
            execvp(argv[2], argv + 2);
        } else {
            close(connection);
        }
    }
    close(sock);
}