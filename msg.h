#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "while_true.h"

struct msg_t {
    char*  text;
    size_t size;
    size_t max_size;
};

struct msg_t create_msg(const size_t max_size) {
    struct msg_t msg = {.text = NULL, .size = 0, .max_size = 0};
    msg.text         = calloc(max_size, sizeof(char));

    if (!msg.text) {
        printf("Creating of msg_t failed");
        return msg;
    }

    msg.max_size = max_size;
    return msg;
}

int recreate_msg(struct msg_t* msg, const size_t new_max_size) {
    if (new_max_size <= msg->max_size) {
        return 0;
    }

    char* new_buf = calloc(new_max_size, sizeof(char));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return -1;
    }

    if (msg->text) {
        memcpy(new_buf, msg->text, msg->size);
        free(msg->text);
    }

    msg->text     = new_buf;
    msg->max_size = new_max_size;
    return 1;
}

int add_text_to_message(struct msg_t* msg, const char* buf, size_t size) {
    size_t new_size = msg->size + size;
    if (new_size > msg->max_size) {
        if (recreate_msg(msg, new_size + 50) < 0) {
            return -1;
        }
    }

    memcpy(msg->text + msg->size, buf, size);
    msg->size = new_size;
    return msg->size;
}

void free_msg(struct msg_t* msg) {
    free(msg->text);
}

struct msg_t recv_one_message(int fd) {
    struct msg_t msg = create_msg(50);
    char         recv_buffer[1000];
    size_t       received = 0;
    while (while_true) {
        int recv_res = recv(fd, &recv_buffer, sizeof(recv_buffer), 0);
        if (recv_res == 0) {
            printf("The connection is closed\n");
            return msg;
        }
        if (recv_res > 0) {
            received += recv_res;
            if (add_text_to_message(&msg, recv_buffer, recv_res) < 0) {
                return msg;
            }
            continue;
        }

        if (errno != EWOULDBLOCK) {
            perror("recv() failed");
            return msg;
        }

        if (received) {
            return msg;
        }
        sleep(0.1);
    }

    return msg;
}
