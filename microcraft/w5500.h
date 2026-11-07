#ifndef W5500_H
#define W5500_H

#include <stdint.h>



enum {
    W5500_OK = 0,
    W5500_ERR_IO = -1,
    W5500_ERR_TIMEOUT = -2,
    W5500_ERR_STATE = -3,
    W5500_ERR_ARG = -4,
    W5500_ERR_VERSION = -5
};

enum {
    W5500_SOCK_CLOSED = 0x00,
    W5500_SOCK_INIT = 0x13,
    W5500_SOCK_LISTEN = 0x14,
    W5500_SOCK_ESTABLISHED = 0x17,
    W5500_SOCK_CLOSE_WAIT = 0x1c
};



typedef struct {
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t subnet[4];
    uint8_t gateway[4];
} w5500_network_t;



int16_t w5500_init(const w5500_network_t *network);
int16_t w5500_link_up(void);

/* ==================== Socket TCP ==================== */
int16_t w5500_socket_status(void);
int16_t w5500_tcp_listen(uint16_t port);
int16_t w5500_tcp_close(void);
int16_t w5500_tcp_disconnect(void);
int16_t w5500_rx_available(void);

int16_t w5500_tcp_recv(uint8_t *buffer, uint16_t capacity);
int16_t w5500_tcp_send(const uint8_t *data, uint16_t len);

#endif
