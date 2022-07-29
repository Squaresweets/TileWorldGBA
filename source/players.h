#ifndef PLAYERS_H   /* Include guard */
#define PLAYERS_H
#include <tonc.h>

void UpdatePlayer(u8* packet);
void PlayerLeave(u32 id);
void InitAllPlayers();
void RenderAllPlayers(BG_POINT bg0_pt);

#endif