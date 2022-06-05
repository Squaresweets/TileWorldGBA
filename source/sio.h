#ifndef SIO_H   /* Include guard */
#define SIO_H

#include <tonc.h>
void sioInit();
void handle_serial();
void loadChunks();
void sioMove();
void processData();
#endif