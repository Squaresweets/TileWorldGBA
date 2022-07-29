#ifndef MAIN_H   /* Include guard */
#define MAIN_H

#include <tonc.h>
void resetPlayerPos();
extern bool startMovement, placeMode;
extern int playerx, playery, camerax, cameray, xv, yv;
extern u16 pingtimer;
extern OBJ_ATTR obj_buffer[128];
#endif