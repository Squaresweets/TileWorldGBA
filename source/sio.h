#ifndef SIO_H   /* Include guard */
#define SIO_H

#include <tonc.h>
void sioInit();
void sioMove();
void ping();
void processData();
void handle_serial();
void requestChunks(int xDir, int yDir);
void place(s32 x, s32 y, u8 ID);
#endif