#ifndef MC_PACKET_H
#define MC_PACKET_H

#include <stddef.h>

int mc_read_varint(int sock, int *value);
int mc_write_varint(unsigned char *buf, int value);

unsigned char *mc_read_packet(int sock, int *packet_size);
int mc_write_string(unsigned char *buf, const char *str);

#endif
