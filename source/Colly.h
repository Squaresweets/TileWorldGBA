#ifndef COLLY_H   /* Include guard */
#define COLLY_Hv

#include <stdbool.h>

typedef struct Vector {
    //All fixed point (<< 16)
    int x;
    int y;
    int w;
    int h;
} vector;
typedef struct CheckReturn {
    vector v;
    bool collided;
} checkreturn;

checkreturn Check(vector bounds, vector goal);

#endif