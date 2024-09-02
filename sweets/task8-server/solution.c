#include <arpa/inet.h>
#include <inttypes.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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

int main(int argc, char *argv[]) {
    int sock = create_listener(argv[1]);
    if (sock < 0) {
        return 1;
    }
    int values_sum = 0;
    int value = 1;
    while (value != 0) {
        int connection = accept(sock, NULL, NULL);
        recv(connection, &value, sizeof(value), MSG_NOSIGNAL);
        value = ntohl(value);
        values_sum += value;
        close(connection);
    }
    close(sock);
    printf("%" PRId32 "\n", values_sum);
}