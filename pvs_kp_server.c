#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "msg.h"

#define DEFAULT_PORT          49001
#define CONNECTION_QUEUE_SIZE 100

int main(int argc, char* argv[]) {
    int listener_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listener_fd < 0) {
        perror("listen_fd socket failed");
        return 0;
    }

    do {
        // Для повторного использования локального адреса при перезапуске
        // сервера до истечения требуемого времени ожидания.
        int on = 1;
        if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on,
                       sizeof(on)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
            break;
        }

        // Сделаем сокет не блокируемым.
        if (ioctl(listener_fd, FIONBIO, (char*)&on) < 0) {
            perror("ioctl() failed");
            break;
        }

        struct sockaddr_in6 listener_addr;
        memset(&listener_addr, 0, sizeof(listener_addr));
        listener_addr.sin6_family = AF_INET6;
        if (argc > 1) {
            listener_addr.sin6_port = htons(atoi(argv[1]));
        } else {
            listener_addr.sin6_port = htons(DEFAULT_PORT);
        }
        if (bind(listener_fd, (struct sockaddr*)&listener_addr,
                 sizeof(listener_addr)) < 0) {
            perror("bind() failed");
            break;
        }

        if (listen(listener_fd, CONNECTION_QUEUE_SIZE) < 0) {
            perror("listen() failed");
            break;
        }

        struct pollfd fds[200];
        int           nfds = 1;
        memset(fds, 0, sizeof(fds));

        // Настройка начального прослушивающего сокета.
        fds[0].fd     = listener_fd;
        fds[0].events = POLLIN;
        // Тайм-аут 3 минуты в мс.
        int timeout    = 3 * 60 * 1000;
        int end_server = 0;
        do {
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
            int current_size = nfds;
            int compress     = 0;
            for (int i = 0; i < current_size; i++) {
                if (fds[i].revents == 0)
                    continue;

                if (fds[i].revents != POLLIN) {
                    printf("  Error! revents = %d\n", fds[i].revents);
                    end_server = 1;
                    break;
                }
                if (fds[i].fd == listener_fd) {
                    printf("  Listening socket is readable\n");

                    // Обработка всех запросов подключения.
                    int client_fd;
                    do {
                        client_fd = accept(listener_fd, NULL, NULL);
                        if (client_fd < 0) {
                            if (errno != EWOULDBLOCK) {
                                perror("  accept() failed");
                                end_server = 1;
                            }
                            break;
                        }

                        // Сделаем сокет не блокируемым.
                        if (ioctl(client_fd, FIONBIO, (char*)&on) < 0) {
                            perror("client_fd ioctl() failed");
                            close(client_fd);
                            break;
                        }

                        printf("  New incoming connection - %d\n", client_fd);
                        fds[nfds].fd     = client_fd;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    } while (client_fd != -1);
                } else {
                    printf("  Descriptor %d is readable\n", fds[i].fd);
                    int close_conn = 0;
                    do {
                        struct msg_t msg = recv_one_message(fds[i].fd);
                        if (!msg.size) {
                            close_conn = 1;
                            free_msg(&msg);
                            break;
                        }

                        printf("  %ld bytes received:\n", msg.size);
                        printf("   -> %s\n", msg.text);
                        // Отправка данных обратно на сервер.
                        int send_res = send(fds[i].fd, msg.text, msg.size, 0);
                        free_msg(&msg);
                        if (send_res < 0) {
                            perror("  send() failed");
                            close_conn = 1;
                            break;
                        }
                    } while (0);

                    // Если соединение прервалось, нужно удалить дескриптор.
                    if (close_conn) {
                        close(fds[i].fd);
                        fds[i].fd = -1;
                        compress  = 1;
                    }
                }
            }

            if (compress) {
                compress = 0;
                for (int i = 0; i < nfds; i++) {
                    if (fds[i].fd == -1) {
                        for (int j = i; j < nfds; j++) {
                            fds[j].fd = fds[j + 1].fd;
                        }
                        i--;
                        nfds--;
                    }
                }
            }

        } while (end_server == 0);

        for (int i = 1; i < nfds; i++) {
            if (fds[i].fd >= 0)
                close(fds[i].fd);
        }

    } while (0);

    close(listener_fd);

    return 0;
}