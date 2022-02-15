#ifndef COLLY_H   /* Include guard */
#define COLLY_H

typedef struct Vector {
    float x;
    float y;
    float w;
    float h;
} vector;

vector Check(vector bounds, vector goal);

#endif