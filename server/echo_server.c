#include "echo_server.h"

#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "msg.h"
#include "while_true.h"

int process_listener(const int listener_fd, struct poll_fd_storage* storage) {
    int client_fd;
    WHILE_TRUE() {
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
        struct pollfd client_poll_fd;
        client_poll_fd.fd     = client_fd;
        client_poll_fd.events = POLLIN;
        if (add_poll_fd_to_storage(storage, client_poll_fd) < 0) {
            return -1;
        }
    };

    return 1;
}

int process_client(const int client_fd) {
    struct msg msg = recv_one_message(client_fd);
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

int process_poll_fds(struct poll_fd_storage* storage) {
    // Есть сокеты, готовые к чтению.
    int current_size = storage->size;
    int compress     = 0;
    for (int i = 0; i < current_size; i++) {
        if (storage->fds[i].revents == 0)
            continue;

        if (storage->fds[i].revents != POLLIN) {
            printf("  Error on descriptor %d! revents = %d\n",
                   storage->fds[i].fd, storage->fds[i].revents);
            close(storage->fds[i].fd);
            storage->fds[i].fd = -1;
            compress           = 1;
            continue;
        }

        if (i == 0) {
            printf("  Listening socket is readable\n");

            if (process_listener(storage->fds[0].fd, storage) < 0) {
                return -1;
            }
            continue;
        }

        printf("  Descriptor %d is readable\n", storage->fds[i].fd);
        if (process_client(storage->fds[i].fd) < 0) {
            close(storage->fds[i].fd);
            storage->fds[i].fd = -1;
            compress           = 1;
        }
    }

    if (compress) {
        compress = 0;
        compress_poll_fd_storage(storage);
    }

    return 1;
}

void echo_server(const int listener_fd) {
    struct poll_fd_storage storage = create_poll_fd_storage(50);
    if (!storage.fds) {
        return;
    }

    // Настройка начального прослушивающего сокета.
    struct pollfd listener_poll_fd;
    listener_poll_fd.fd     = listener_fd;
    listener_poll_fd.events = POLLIN;

    if (add_poll_fd_to_storage(&storage, listener_poll_fd) < 0) {
        return;
    }

    // Тайм-аут 3 минуты в мс.
    int timeout = 3 * 60 * 1000;
    WHILE_TRUE() {
        printf("Waiting on poll()...\n");
        int poll_res = poll(storage.fds, storage.size, timeout);

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
        if (process_poll_fds(&storage) < 0) {
            break;
        }
    };

    for (size_t i = 1; i < storage.size; i++) {
        if (storage.fds[i].fd >= 0)
            close(storage.fds[i].fd);
    }
    free_poll_fd_storage(&storage);
}