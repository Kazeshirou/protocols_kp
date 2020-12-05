#include "echo_client.h"

#include <errno.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "msg.h"
#include "while_true.h"

void echo_client(int client_fd) {
    char buffer[1000];

    struct pollfd fds[2];
    // Чтение со стандартного ввода.
    fds[0].fd     = 0;
    fds[0].events = POLLIN;
    // Сообщение от сервера.
    fds[1].fd     = client_fd;
    fds[1].events = POLLIN;
    // Тайм-аут 3 минуты в мс.
    int timeout = 3 * 60 * 1000;
    WHILE_TRUE() {
        printf("Waiting on poll()...\n");
        int poll_res = poll(fds, 2, timeout);

        if (poll_res < 0) {
            perror("  poll() failed");
            return;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            printf("  poll() timed out.  End program.\n");
            return;
        }

        if (fds[0].revents) {
            if (fds[0].revents != POLLIN) {
                printf("  Unexpected error on input! revents = %d\n",
                       fds[0].revents);
                return;
            }
            // Со стандартного ввода можно считать данные.
            size_t length = read(fds[0].fd, buffer, sizeof(buffer) - 1);
            printf("  %ld bytes read\n", length);
            if (!length) {
                return;
            }
            if (buffer[length - 1] == '\n') {
                length--;
            }
            buffer[length] = 0;
            printf("   -> %s\n", buffer);
            int send_res;
            do {
                send_res = send(client_fd, buffer, length, 0);
            } while ((send_res < 0) && (errno == EWOULDBLOCK));
            if (send_res < 0) {
                perror("send() failed");
                return;
            }
        }

        if (fds[1].revents) {
            if (fds[1].revents != POLLIN) {
                printf("  Unexpected error on input! revents = %d\n",
                       fds[0].revents);
                return;
            }

            struct msg msg = recv_one_message(client_fd);
            printf("  %ld bytes received from server\n", msg.size);
            if (msg.size) {
                printf("   -> %s\n", msg.text);
                free_msg(&msg);
            } else {
                return;
            }
        }
    }
}