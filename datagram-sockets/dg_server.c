#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "4950"
#define MAXBUFFER 100


void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return  &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void config_getaddrinfo(int* rv, struct addrinfo** servinfo) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((*rv = getaddrinfo(NULL, PORT, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(*rv));
        exit(1);
    }
    else printf("[OK] getaddrinfo success\n");
}

void config_bind(struct addrinfo** p, struct addrinfo* servinfo, int* sockfd) {
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
        if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(*sockfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
            close(*sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
}

void check_bind(struct addrinfo* p) {
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        exit(2);
    }
    else printf("[OK] bind success\n");
    printf("server: waiting to receive\n");

}

void config_reciver(int* sockfd) {
    struct sockaddr_storage addr_storage;
    socklen_t addr_len = sizeof addr_storage;
    int numbytes;
    char buf[MAXBUFFER];
    char s[INET6_ADDRSTRLEN];

    if ((numbytes = recvfrom(*sockfd, buf, MAXBUFFER - 1, 0, (struct sockaddr*)&addr_storage, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    inet_ntop(addr_storage.ss_family, get_in_addr((struct sockaddr*)&addr_storage), s, sizeof s);
    printf("server: got packet from %s\n", s);
    printf("server: size of packet => %d bytes\n", numbytes);
    buf[numbytes] = '\0';
    printf("server: data in packet => %s\n", buf);
    close(*sockfd);
}

int main() {
    int sockfd;
    struct addrinfo* servinfo, * p;
    int rv;

    config_getaddrinfo(&rv, &servinfo);
    config_bind(&p, servinfo, &sockfd);
    check_bind(p);
    freeaddrinfo(servinfo);
    config_reciver(&sockfd);

    return 0;
}