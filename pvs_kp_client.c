#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "msg.h"

#define DEFAULT_SERVER_PORT "49001"


int echo_client(int client_fd) {
    char buffer[250];
    memset(buffer, 0, sizeof(buffer));

    fgets(buffer, sizeof(buffer) - 1, stdin);
    size_t length = strlen(buffer) - 1;
    if (!length) {
        return 0;
    }
    buffer[length] = 0;
    int send_res;
    do {
        send_res = send(client_fd, buffer, length, 0);
    } while ((send_res < 0) && (errno == EWOULDBLOCK));
    if (send_res < 0) {
        perror("send() failed");
        return -1;
    }

    struct msg_t msg = recv_one_message(client_fd);
    if (msg.size) {
        printf("%s\n", msg.text);
        free_msg(&msg);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    char server[100];
    if (argc > 1) {
        strcpy(server, argv[1]);
    } else {
        perror("Server addr is undefined");
        return 0;
    }

    struct addrinfo hints;
    memset(&hints, 0x00, sizeof(hints));
    hints.ai_flags    = AI_NUMERICSERV;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct in6_addr serveraddr;
    if (inet_pton(AF_INET, server, &serveraddr) == 1) {
        // IPv4
        hints.ai_family = AF_INET;
        hints.ai_flags |= AI_NUMERICHOST;
    } else if (inet_pton(AF_INET6, server, &serveraddr) == 1) {
        hints.ai_family = AF_INET6;
        hints.ai_flags |= AI_NUMERICHOST;
    }

    int              getaddr_res;
    struct addrinfo* res = NULL;
    if ((getaddr_res =
             getaddrinfo(server, DEFAULT_SERVER_PORT, &hints, &res)) != 0) {
        printf("Host not found --> %s\n", gai_strerror(getaddr_res));
        if (getaddr_res == EAI_SYSTEM) {
            perror("getaddrinfo() failed");
        }
        if (res != NULL) {
            freeaddrinfo(res);
        }
        return 0;
    }

    int client_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_fd < 0) {
        perror("socket() failed");
        if (res != NULL) {
            freeaddrinfo(res);
        }
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
    if (res != NULL) {
        freeaddrinfo(res);
    }
}