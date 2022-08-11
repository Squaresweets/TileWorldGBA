#ifndef MAP_H   /* Include guard */
#define MAP_H
#include <tonc.h>

extern u8 map[28800];
extern int mapOffsetX;
extern int mapOffsetY;
extern int mapX;
extern int mapY;
void loadChunks();
void setupMap();
void processNewChunkData(u32 data, u32 offset);
void setTile(u32 x, u32 y, u8 id);
void processNewChunk();

void EnableMiniMapMode();
#endif