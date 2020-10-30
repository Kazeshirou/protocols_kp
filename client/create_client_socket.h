#pragma once

#include <netdb.h>

int create_client_socket(const struct addrinfo* server_addr);