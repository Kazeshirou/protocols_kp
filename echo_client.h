#pragma once

#include <sys/poll.h>

#include "msg.h"
#include "while_true.h"

void echo_client(int client_fd) {
    char buffer[1000];

    struct pollfd fd;
    fd.fd     = 0;
    fd.events = POLLIN;
    // Тайм-аут 3 минуты в мс.
    int timeout = 3 * 60 * 1000;
    while (while_true) {
        int poll_res = poll(&fd, 1, timeout);

        if (poll_res < 0) {
            perror("  poll() failed");
            return;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            printf("  poll() timed out.  End program.\n");
            return;
        }

        if (fd.revents != POLLIN) {
            printf("  Unexpected error on input! revents = %d\n", fd.revents);
            return;
        }

        size_t length = read(fd.fd, buffer, sizeof(buffer) - 1) - 1;
        if (!length) {
            return;
        }

        buffer[length] = 0;
        int send_res;
        do {
            send_res = send(client_fd, buffer, length, 0);
        } while ((send_res < 0) && (errno == EWOULDBLOCK));
        if (send_res < 0) {
            perror("send() failed");
            return;
        }

        struct msg_t msg = recv_one_message(client_fd);
        if (msg.size) {
            printf("%s\n", msg.text);
            free_msg(&msg);
        }
    }
}