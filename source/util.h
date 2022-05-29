#ifndef UTIL_H   /* Include guard */
#define UTIL_H

#define SHIFT_AMOUNT 16
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define DECIMAL_MASK ((1 << SHIFT_AMOUNT) - 1)
#define INT_MASK ~((1 << SHIFT_AMOUNT) - 1)
#define Fixed_to_float(X) ((float)X / ONE_SHIFTED)

float ReverseFloat( const float inFloat );
#endif