#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>

struct poll_fd_storage {
    struct pollfd* fds;
    size_t         size;
    size_t         max_size;
};

struct poll_fd_storage create_poll_fd_storage(const size_t max_size) {
    struct poll_fd_storage storage = {.fds = NULL, .size = 0, .max_size = 0};
    storage.fds = (struct pollfd*)calloc(max_size, sizeof(struct pollfd));

    if (!storage.fds) {
        printf("Creating of poll_fd_storage failed");
        return storage;
    }

    storage.max_size = max_size;
    for (int i = 0; i < max_size; i++) {
        storage.fds[i].fd = -1;
    }
    return storage;
}

int recreate_poll_fd_storage(struct poll_fd_storage* storage,
                             const size_t            new_max_size) {
    if (new_max_size <= storage->max_size) {
        return 0;
    }

    struct pollfd* new_buf = (struct pollfd*)calloc(new_max_size, sizeof(char));
    if (!new_buf) {
        printf("new_buf calloc failed");
        return -1;
    }
    for (int i = storage->max_size; i < new_max_size; i++) {
        new_buf[i].fd = -1;
    }
    if (storage->fds) {
        memcpy(new_buf, storage->fds, storage->size);
        free(storage->fds);
    }

    storage->fds      = new_buf;
    storage->max_size = new_max_size;
    return 1;
}

int add_poll_fd_to_storage(struct poll_fd_storage* storage,
                           const struct pollfd     fd) {
    size_t new_size = storage->size + 1;
    if (new_size > storage->max_size) {
        if (recreate_poll_fd_storage(storage, new_size + 50) < 0) {
            return -1;
        }
    }

    storage->fds[storage->size] = fd;
    storage->size               = new_size;
    return storage->size;
}

void free_poll_fd_storage(struct poll_fd_storage* storage) {
    free(storage->fds);
}

void compress_poll_fd_storage(struct poll_fd_storage* storage) {
    for (int i = 0; i < storage->size; i++) {
        if (storage->fds[i].fd == -1) {
            for (int j = i; j < storage->size; j++) {
                storage->fds[j].fd = storage->fds[j + 1].fd;
            }
            i--;
            storage->size--;
        }
    }
}