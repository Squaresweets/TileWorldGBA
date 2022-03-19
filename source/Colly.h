#ifndef COLLY_H   /* Include guard */
#define COLLY_H

typedef struct Vector {
    //All fixed point (<< 16)
    int x;
    int y;
    int w;
    int h;
} vector;

vector Check(vector bounds, vector goal);

#endif