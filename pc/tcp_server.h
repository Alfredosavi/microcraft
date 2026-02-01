#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stddef.h>

int tcp_server_create(int port);
int tcp_server_accept(int server_fd);

int tcp_read(int sock, void *buffer, size_t size);
int tcp_write(int sock, const void *buffer, size_t size);

void tcp_server_close(int sock);

#endif
