#ifndef MC_PACKETS_H
#define MC_PACKETS_H

#include <stdint.h>

int16_t mc_send_status(void);
int16_t mc_send_pong(const uint8_t payload[8]);
int16_t mc_send_login(const uint8_t *name, uint8_t length);

#endif
