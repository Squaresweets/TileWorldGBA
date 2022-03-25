#ifndef COLLY_H   /* Include guard */
#define COLLY_H

#include <tonc.h>

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
    bool ladder;
} checkreturn;

checkreturn Check(vector bounds, vector goal);

 //https://stackoverflow.com/questions/3437404/min-and-max-in-c
 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#endif