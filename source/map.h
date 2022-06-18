#ifndef MAP_H   /* Include guard */
#define MAP_H
#include <tonc.h>

extern u8 map[28800];
extern int MapOffsetX, mapOffsetY, mapX, mapY;
extern volatile bool setupmapTrigger;
void loadChunks();
void setupMap();


#endif