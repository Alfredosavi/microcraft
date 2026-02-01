#ifndef CLIENT_H
#define CLIENT_H

enum {
    STATE_HANDSHAKE,
    STATE_STATUS,
    STATE_LOGIN,
    STATE_PLAY
};

typedef struct {
    int socket;
    int state;
} Client;

void client_init(Client *c, int sock);
void client_handle_packet(Client *c, unsigned char *data, int size);

#endif
