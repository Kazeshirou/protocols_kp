#include <stdio.h>
#include <string.h>  // strcpy
#include <unistd.h>  // close

#include "create_client_socket.h"
#include "echo_client.h"
#include "end_marker.h"
#include "end_program_handler.h"
#include "get_server_addr.h"
#include "while_true.h"

#define DEFAULT_SERVER_PORT "49001"

int         while_true = 1;
const char* end_marker = "123";

int main(int argc, char* argv[]) {
    if (set_end_program_handler() < 0) {
        return 0;
    }

    char server[100];
    char port[20];
    if (argc > 1) {
        strcpy(server, argv[1]);
    } else {
        printf("Server addr is undefined");
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

    echo_client(client_fd);

    close(client_fd);
    freeaddrinfo(server_addr);
}