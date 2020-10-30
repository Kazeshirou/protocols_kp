#pragma once

#include "poll_fd_storage.h"

int  process_listener(const int listener_fd, struct poll_fd_storage* storage);
int  process_client(const int client_fd);
int  process_poll_fds(struct poll_fd_storage* storage);
void echo_server(const int listener_fd);