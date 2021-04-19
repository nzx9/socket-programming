#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>

#define PORT "3490"
#define BACKLOG 10


void sigchld_handler() {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void config_getaddrinfo(struct addrinfo hints, struct addrinfo** servinfo, int* rv) {
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((*rv = getaddrinfo(NULL, PORT, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(*rv));
        exit(1);
    }
    else printf("[OK] addrinfo success\n");
}

void config_bind(struct addrinfo** p, struct addrinfo* servinfo, int* sockfd, int* yes) {
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
        if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype, (*p)->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(*sockfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
            close(*sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
}

void check_server_bind(struct addrinfo* p) {
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    else printf("[OK] binding success\n");

}
void config_listen(int sockfd) {
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    else printf("[OK] listening success\n");

}

void sigaction_handler(struct sigaction sa) {
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");
}

void accept_loop(socklen_t* sin_size, struct sockaddr_storage storage_addr, int* new_fd, int* sockfd, int* bytes_sent) {
    char s[INET6_ADDRSTRLEN];
    while (1) {
        *sin_size = sizeof storage_addr;
        *new_fd = accept(*sockfd, (struct sockaddr*)&storage_addr, sin_size);
        if (*new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(storage_addr.ss_family, get_in_addr((struct sockaddr*)&storage_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) {
            close(*sockfd);
            char msg[] = "Hello World!";
            if ((*bytes_sent += send(*new_fd, msg, sizeof msg, 0)) == -1) perror("send");
            printf("sent: %d bytes\n", *bytes_sent);
            close(*new_fd);
            exit(0);
        }
        close(*new_fd);
    }

}

int main() {
    int sockfd, new_fd;
    struct addrinfo hints, * servinfo, * p = NULL;
    struct sockaddr_storage storage_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    int rv;
    int bytes_sent = 0;

    config_getaddrinfo(hints, &servinfo, &rv);
    config_bind(&p, servinfo, &sockfd, &yes);
    freeaddrinfo(servinfo);
    check_server_bind(p);
    config_listen(sockfd);
    sigaction_handler(sa);
    accept_loop(&sin_size, storage_addr, &new_fd, &sockfd, &bytes_sent);

}