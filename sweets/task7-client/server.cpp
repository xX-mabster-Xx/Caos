#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>

void alarm_handler(int s)
{
    exit(0);
}

std::map<unsigned long long, std::pair<int, unsigned long long> > keys
{
    { 0, { 5, 25 } },
    { 207395599086893, { 1, 205263323535775 } },
    { 45046078115294, { 5, 136594820654008 } },
    { 259285561181631, { 100, 120714737763871 } },
    { 117292971162708, { 10000, 204931170357017 } },
    { 128744587772120, { 10000, 0 } },
    { 204796033187209, { 10000, 1 } },
    { 175121057857852, { 2, 2 } },
};

void dochild(int fd, const char *clientid)
{
    char key[1024];

    FILE *fin = fdopen(fd, "r");
    FILE *fout = fdopen(dup(fd), "w");

    signal(SIGALRM, alarm_handler);
    alarm(10);

    if (fscanf(fin, "%1023s", key) != 1) {
        fprintf(stderr, "%s: protocol error: no key\n", clientid);
        fclose(fout);
        fclose(fin);
        return;
    }
    char *eptr = NULL;
    errno = 0;
    unsigned long long keyval = strtoull(key, &eptr, 10);
    if (errno || *eptr) {
        fprintf(stderr, "%s: protocol error: invalid key\n", clientid);
        fclose(fout);
        fclose(fin);
        return;
    }
    auto kit = keys.find(keyval);
    if (kit == keys.end()) {
        fprintf(stderr, "%s: key %llu not supported\n", clientid, keyval);
        fclose(fout);
        fclose(fin);
        return;
    }
    int maxval = kit->second.first;
    unsigned long long retval = kit->second.second;
    if (retval == 0) {
        fclose(fout);
        fclose(fin);
        exit(0);
    }
    fprintf(fout, "%d\n", maxval);
    fflush(fout);
    if (retval == 1) {
        fclose(fout);
        fclose(fin);
        exit(0);
    }
    for (int z = 0; z <= maxval; ++z) {
        int zz;
        if (fscanf(fin, "%d", &zz) != 1) {
            fprintf(stderr, "%s:%llu: failed to read value %d\n", clientid, keyval, z);
            fclose(fout);
            fclose(fin);
            return;
        }
        if (zz != z) {
            fprintf(stderr, "%s:%llu: wrong value read: %d instead of %d\n", clientid, keyval, zz, z);
            fclose(fout);
            fclose(fin);
            return;
        }
    }
    if (retval == 2) {
        fclose(fout);
        fclose(fin);
        exit(0);
    }
    fprintf(fout, "%llu\n", retval);
    fflush(fout);
    fclose(fout);
    fclose(fin);
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments\n");
        return 1;
    }
    int port = strtol(argv[1], NULL, 10);
    int afd = socket(PF_INET, SOCK_STREAM, 0);
    if (afd < 0) {
        fprintf(stderr, "socket() failed: %s\n", strerror(errno));
        return 1;
    }

    int val = 1;
    setsockopt(afd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    setsockopt(afd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);
    if (bind(afd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)) < 0) {
        fprintf(stderr, "bind() failed: %s\n", strerror(errno));
        return 1;
    }

    if (listen(afd, 5) < 0) {
        fprintf(stderr, "listen() failed: %s\n", strerror(errno));
        return 1;
    }

    signal(SIGCHLD, SIG_IGN);

    while (1) {
        char clientid[256];
        struct sockaddr_in ss;
        socklen_t sz = sizeof(ss);
        int fd = accept(afd, reinterpret_cast<struct sockaddr *>(&ss), &sz);
        if (fd < 0) {
            fprintf(stderr, "accept() failed: %s\n", strerror(errno));
            continue;
        }
        snprintf(clientid, sizeof(clientid), "%s:%d", inet_ntoa(ss.sin_addr), ntohs(ss.sin_port));
        fprintf(stderr, "connect from %s\n", clientid);
        fflush(stdout);
        int pid = fork();
        if (pid < 0) {
            fprintf(stderr, "fork() failed: %s\n", strerror(errno));
            close(fd);
            continue;
        }
        if (!pid) {
            close(afd);
            dochild(fd, clientid);
            exit(0);
        }
        close(fd);
    }
}
