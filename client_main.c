
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "echo_client.h"
#include "get_server_addr.h"

#define DEFAULT_SERVER_PORT "49001"

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

int main(int argc, char* argv[]) {
    char server[100];
    char port[20];
    if (argc > 1) {
        strcpy(server, argv[1]);
    } else {
        perror("Server addr is undefined");
        return 0;
    }

    if (argc > 2) {
        strcpy(port, argv[2]);
    } else {
        strcpy(port, DEFAULT_SERVER_PORT);
    }

    struct addrinfo* server_addr = get_server_address(server, port);
    if (!server_addr) {
        return 0;
    }

    int client_fd = create_client_socket(server_addr);
    if (client_fd < 0) {
        freeaddrinfo(server_addr);
        return 0;
    }

    while (echo_client(client_fd) > 0) {
        ;
    }

    close(client_fd);
    freeaddrinfo(server_addr);
}