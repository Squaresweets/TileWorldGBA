#ifndef UTIL_H   /* Include guard */
#define UTIL_H

#include <tonc.h>

#define SHIFT_AMOUNT 12
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define DECIMAL_MASK ((1 << SHIFT_AMOUNT) - 1)
#define INT_MASK ~((1 << SHIFT_AMOUNT) - 1)
#define Fixed_to_float(X) ((float)X / ONE_SHIFTED)
#define Float_to_fixed(x) ((int)((x) * ONE_SHIFTED)) 

#define INITIAL_PLAYER_POS (32 << SHIFT_AMOUNT)

#define SCREEN_W      240
#define SCREEN_H      160

u32 se_index(u32 tx, u32 ty, u32 pitch);
s32 map_index(s32 tx, s32 ty);
u32 Reverse32(u32 value);
u32 ReverseNibbles32(u32 value);
#endif