#ifndef MC_SERVER_H
#define MC_SERVER_H

#include <stdbool.h>
#include <stdint.h>

enum { MC_ERR_PACKET = -10, MC_ERR_VERSION = -11, MC_ERR_TIMEOUT = -12 };



void mc_server_reset(void);
int16_t mc_server_poll(void);
int16_t mc_server_toggle_devices(void);
bool mc_server_at_boundary(void);

#endif
