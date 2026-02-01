#ifndef PLAY_H
#define PLAY_H

#include "client.h"

void play_send_join_game(Client *c);
void play_send_player_pos(Client *c);

#endif