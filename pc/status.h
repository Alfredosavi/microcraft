#ifndef STATUS_H
#define STATUS_H

#include "client.h"

void status_send_response(Client *c);
void status_handle_ping(Client *c, unsigned char *data, int size, int id_len);

#endif
