#ifndef PLAYERS_H   /* Include guard */
#define PLAYERS_H
#include <tonc.h>

void updatePlayer(u8* packet);
void PlayerLeave(u32 id);
void MovePlayers();

#endif