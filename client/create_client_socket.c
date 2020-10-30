#include "create_client_socket.h"

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_client_socket(const struct addrinfo* server_addr) {
    int client_fd = socket(server_addr->ai_family, server_addr->ai_socktype,
                           server_addr->ai_protocol);
    if (client_fd < 0) {
        perror("socket() failed");
        return -1;
    }

    int on = 1;
    // Сделаем сокет не блокируемым.
    if (ioctl(client_fd, FIONBIO, (char*)&on) < 0) {
        perror("ioctl() failed");
        close(client_fd);
        return -1;
    }

    int connect_res;
    do {
        connect_res =
            connect(client_fd, server_addr->ai_addr, server_addr->ai_addrlen);
    } while ((connect_res < 0) &&
             ((errno == EINPROGRESS) || (errno == EWOULDBLOCK)));

    if (connect_res < 0) {
        perror("connect() failed");
        close(client_fd);
        return -1;
    }

    return client_fd;
}