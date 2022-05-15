#ifndef MAIN_H   /* Include guard */
#define MAIN_H

#include <tonc.h>

void resetPlayerPos();
extern bool startMovement;
extern int playerx, playery, camerax, cameray, xv, yv;
int mod(int x,int N);

#define SHIFT_AMOUNT 16
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define DECIMAL_MASK ((1 << SHIFT_AMOUNT) - 1)
#define INT_MASK ~((1 << SHIFT_AMOUNT) - 1)
#define Fixed_to_float(X) ((float)X / ONE_SHIFTED)
#endif