#pragma once

#include "msg.h"

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