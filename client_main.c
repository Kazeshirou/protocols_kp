
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "echo_client.h"
#include "get_server_addr.h"

#define DEFAULT_SERVER_PORT "49001"


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

    struct addrinfo* res = get_server_address(server, port);
    if (!res) {
        return 0;
    }

    int client_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_fd < 0) {
        perror("socket() failed");
        freeaddrinfo(res);
        return 0;
    }

    do {
        int on = 1;
        // Сделаем сокет не блокируемым.
        if (ioctl(client_fd, FIONBIO, (char*)&on) < 0) {
            perror("ioctl() failed");
            break;
        }

        int connect_res;
        do {
            connect_res = connect(client_fd, res->ai_addr, res->ai_addrlen);
        } while ((connect_res < 0) &&
                 ((errno == EINPROGRESS) || (errno == EWOULDBLOCK)));

        if (connect_res < 0) {
            perror("connect() failed");
            break;
        }

        while (echo_client(client_fd) > 0) {
            ;
        }
    } while (0);

    close(client_fd);
    freeaddrinfo(res);
}