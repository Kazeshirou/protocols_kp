#pragma once

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "msg.h"

void compress_poll_fds(struct pollfd* fds, int* nfds) {
    for (int i = 0; i < *nfds; i++) {
        if (fds[i].fd == -1) {
            for (int j = i; j < *nfds; j++) {
                fds[j].fd = fds[j + 1].fd;
            }
            i--;
            (*nfds)--;
        }
    }
}

int process_listener(const int listener_fd, struct pollfd* fds, int* nfds) {
    int client_fd;
    while (1) {
        client_fd = accept(listener_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno != EWOULDBLOCK) {
                perror("  accept() failed");
                return -1;
            }
            break;
        }

        int on = 1;
        // Сделаем сокет не блокируемым.
        if (ioctl(client_fd, FIONBIO, (char*)&on) < 0) {
            perror("client_fd ioctl() failed");
            close(client_fd);
            continue;
        }

        printf("  New incoming connection - %d\n", client_fd);
        fds[*nfds].fd     = client_fd;
        fds[*nfds].events = POLLIN;
        (*nfds)++;
    };

    return 1;
}

int process_client(const int client_fd) {
    struct msg_t msg = recv_one_message(client_fd);
    if (!msg.size) {
        free_msg(&msg);
        return -1;
    }

    printf("  %ld bytes received:\n", msg.size);
    printf("   -> %s\n", msg.text);
    // Отправка данных обратно на сервер.
    int send_res = send(client_fd, msg.text, msg.size, 0);
    free_msg(&msg);
    if (send_res < 0) {
        perror("  send() failed");
        return -1;
    }
    return 1;
}

int process_poll_fds(struct pollfd* fds, int* nfds) {
    // Есть сокеты, готовые к чтению.
    int current_size = *nfds;
    int compress     = 0;
    for (int i = 0; i < current_size; i++) {
        if (fds[i].revents == 0)
            continue;

        if (fds[i].revents != POLLIN) {
            printf("  Error! revents = %d\n", fds[i].revents);
            return -1;
        }

        if (i == 0) {
            printf("  Listening socket is readable\n");

            if (process_listener(fds[0].fd, fds, nfds) < 0) {
                return -1;
            }
            continue;
        }

        printf("  Descriptor %d is readable\n", fds[i].fd);
        if (process_client(fds[i].fd) < 0) {
            close(fds[i].fd);
            fds[i].fd = -1;
            compress  = 1;
        }
    }

    if (compress) {
        compress = 0;
        compress_poll_fds(fds, nfds);
    }

    return 1;
}

void echo_server(const int listener_fd) {
    struct pollfd fds[200];
    int           nfds = 1;
    memset(fds, 0, sizeof(fds));

    // Настройка начального прослушивающего сокета.
    fds[0].fd     = listener_fd;
    fds[0].events = POLLIN;
    // Тайм-аут 3 минуты в мс.
    int timeout = 3 * 60 * 1000;
    while (1) {
        printf("Waiting on poll()...\n");
        int poll_res = poll(fds, nfds, timeout);

        if (poll_res < 0) {
            perror("  poll() failed");
            break;
        }
        // Истёк тайм-аут.
        if (poll_res == 0) {
            printf("  poll() timed out.  End program.\n");
            break;
        }

        // Есть сокеты, готовые к чтению.
        if (process_poll_fds(fds, &nfds) < 0) {
            break;
        }
    };

    for (int i = 1; i < nfds; i++) {
        if (fds[i].fd >= 0)
            close(fds[i].fd);
    }
}