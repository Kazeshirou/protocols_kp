#include "create_client_socket.h"
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