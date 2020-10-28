#include "create_server_socket.h"
#include "echo_server.h"

#define DEFAULT_PORT          49001
#define CONNECTION_QUEUE_SIZE 100

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    int listener_fd = create_server_socket(port, CONNECTION_QUEUE_SIZE);
    if (listener_fd < 0) {
        return 0;
    }

    echo_server(listener_fd);

    close(listener_fd);

    return 0;
}