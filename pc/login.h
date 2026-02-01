#ifndef LOGIN_H
#define LOGIN_H

#include "client.h"

void login_handle_start(Client *c, unsigned char *data, int size, int id_len);

#endif
