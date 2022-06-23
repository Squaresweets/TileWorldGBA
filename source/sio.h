#ifndef SIO_H   /* Include guard */
#define SIO_H

#include <tonc.h>
void sioInit();
void handle_serial();
void sioMove();
void processData();
void requestChunks(int xDir, int yDir);
#endif