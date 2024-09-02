#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void PrintAddrInfoByHostAndPort(const char *host, const char *port) {
    // perform address resolution
    struct addrinfo *res = NULL;
    int gai_err;
    struct addrinfo hints = {
        .ai_family = PF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_flags = 0,  // try AI_ALL to include IPv6 on non-v6-enabled systems
    };
    if ((gai_err = getaddrinfo(host, port, &hints, &res))) {
        printf("%s\n", gai_strerror(gai_err));
        return;
    }

    // iterate over the resulting addresses
    struct in_addr output_host;
    uint16_t output_port;
    int first = 1;
    for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
        struct in_addr host = ((struct sockaddr_in *)ai->ai_addr)->sin_addr;
        int16_t port = ((struct sockaddr_in *)ai->ai_addr)->sin_port;

        if (first || ntohl(host.s_addr) < ntohl(output_host.s_addr)) {
            first = 0;
            output_host = host;
            output_port = port;
        }
    }

    char *name = inet_ntoa(output_host);

    printf("%s:%d\n", name, ntohs(output_port));

    freeaddrinfo(res);
}

int main(int argc, char *argv[]) {
    char host[1000], port[1000];
    while (scanf("%s %s", host, port) != EOF) {
        PrintAddrInfoByHostAndPort(host, port);
    }
}
