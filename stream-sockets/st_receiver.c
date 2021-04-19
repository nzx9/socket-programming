#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXDATASIZE 100

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

void config_getaddrinfo(int* rv, char* host, char* port, struct addrinfo** servinfo) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((*rv = getaddrinfo(host, port, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(*rv));
        exit(1);
    }
    else printf("[OK] getaddrinfo success\n");
}

void config_connect(struct addrinfo** p, struct addrinfo* servinfo, int* sockfd) {
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
        if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(*sockfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
            close(*sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
}

void check_connect(struct addrinfo* p) {
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    else printf("[OK] connection success\n");

}

void print_clients(struct addrinfo* p) {
    char s[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
}

void config_recv(int* numbytes, int sockfd) {
    char buf[MAXDATASIZE];
    if ((*numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[*numbytes] = *"\0";
    printf("client: received '%s'\n", buf);
    close(sockfd);
}

int main(int argc, char* args[]) {
    int sockfd, numbytes;
    struct addrinfo* servinfo, * p = NULL;
    int rv;

    if (argc != 3) {
        fprintf(stderr, "usage: client [hostname] [port]\n");
        exit(1);
    }

    config_getaddrinfo(&rv, args[1], args[2], &servinfo);
    config_connect(&p, servinfo, &sockfd);
    check_connect(p);
    print_clients(p);
    freeaddrinfo(servinfo);
    config_recv(&numbytes, sockfd);
    return 0;
}