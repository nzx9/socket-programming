#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

void config_getaddrinfo(int* rv, char* host, char* port, struct addrinfo** servinfo) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((*rv = getaddrinfo(host, port, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(*rv));
        exit(1);
    }
}

void config_socket(struct addrinfo** p, struct addrinfo* servinfo, int* sockfd) {
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
        if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
            perror("receiver: socket");
            continue;
        }
        break;
    }
}

void check_socket(struct addrinfo* p) {
    if (p == NULL) {
        fprintf(stderr, "receiver: failed to create socket\n");
        exit(2);
    }
    else printf("[OK] Socket Created Successfully\n");
}

void handle_send(int* sockfd, char* host, char* port, char* msg, struct addrinfo* p, struct addrinfo* servinfo) {
    int numbytes;
    if ((numbytes = sendto(*sockfd, msg, strlen(msg), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("receiver: sendto");
        exit(1);
    }
    printf("receiver: connected to %s:%s\n", host, port);
    freeaddrinfo(servinfo);
    printf("receiver: %d bytes sent\n", numbytes);
    close(*sockfd);
}

int main(int argc, char* argv[]) {
    int rv, sockfd;
    struct addrinfo* servinfo, * p;
    if (argc != 4) {
        printf("usage: %s [hostname] [port] [msg]\n", argv[0]);
        exit(1);
    }

    config_getaddrinfo(&rv, argv[1], argv[2], &servinfo);
    config_socket(&p, servinfo, &sockfd);
    check_socket(p);
    handle_send(&sockfd, argv[1], argv[2], argv[3], p, servinfo);
    return 0;
}