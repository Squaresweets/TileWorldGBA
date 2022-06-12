#ifndef UTIL_H   /* Include guard */
#define UTIL_H

#include <tonc.h>

#define SHIFT_AMOUNT 16
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define DECIMAL_MASK ((1 << SHIFT_AMOUNT) - 1)
#define INT_MASK ~((1 << SHIFT_AMOUNT) - 1)
#define Fixed_to_float(X) ((float)X / ONE_SHIFTED)

u32 se_index(u32 tx, u32 ty, u32 pitch);
u32 map_index(u32 tx, u32 ty);
u32 Reverse32(u32 value);
#endif