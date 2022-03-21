#include "toolbox.h"
#include "types.h"
#include <stdbool.h>

#include "Colly.h"

 //https://stackoverflow.com/questions/3437404/min-and-max-in-c
 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define mapsize 64

//Fixed point stuff https://stackoverflow.com/questions/10067510/fixed-point-arithmetic-in-c-programming
#define SHIFT_AMOUNT 16
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define DECIMAL_MASK ((1 << SHIFT_AMOUNT) - 1)
#define INT_MASK ~((1 << SHIFT_AMOUNT) - 1)


u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}

bool Intersects(vector *rect, vector *other, vector *intersection)
{
    float interleft = max(rect->x, other->x);
    float intertop = max(rect->y, other->y);
    //incoming vectors always have a size of 1 (either the player or the grid)
    float interright = min(rect->x + ONE_SHIFTED, other->x + ONE_SHIFTED);
    float interbottom = min(rect->y + ONE_SHIFTED, other->y + ONE_SHIFTED);

    if (interleft < interright && intertop < interbottom)
    {
        intersection->x = interleft; intersection->y = intertop;
        intersection->w = interright-interleft; intersection->h = interbottom-intertop;
        return true;
    }
    return false;
}

//Cor thats a lot of arguments
void increment(vector *checkRegion, vector *goal, vector *intersect, vector *bounds, bool X, checkreturn *r)
{
    if(X)
        bounds->x += (goal->x - bounds->x);
    else
        bounds->y += (goal->y - bounds->y);

    for(int y = (checkRegion->y & INT_MASK); y <= (checkRegion->h & INT_MASK); y += ONE_SHIFTED)
    {
        for(int x = (checkRegion->x & INT_MASK); x <= (checkRegion->w & INT_MASK); x += ONE_SHIFTED)
        {
            int id = se_mem[28][se_index(x >> SHIFT_AMOUNT,y >> SHIFT_AMOUNT,mapsize)];

            //If it is air carry on
            if(id == 0 || id == 7) continue;

            vector cell;
            cell.x = x; cell.y = y; cell.w = ONE_SHIFTED; cell.h = ONE_SHIFTED;
            if(Intersects(&cell, bounds, intersect))
            {
                r->collided = true;
                if((X && intersect->w < intersect->h) || (!X && intersect->w > intersect->h))
                {
                    if(X && bounds->x < cell.x) intersect->w *= -1;
                    if(!X && bounds->y < cell.y) intersect->h *= -1;

                    if(X)
                        bounds->x += intersect->w;
                    else
                        bounds->y += intersect->h;
                    break;
                }
            }
        }
    }
}

//We are going to assume steps is always 1
checkreturn Check(vector start, vector end)
{
    vector bounds = start;
    vector goal = end;
    vector intersect;
    checkreturn r;
    r.collided = false;

    vector checkRegion;
    checkRegion.x = max(0, (min(goal.x, bounds.x) - bounds.w) & INT_MASK);
    checkRegion.y = max(0, (min(goal.y, bounds.y) - bounds.h) & INT_MASK);
    checkRegion.w = min((mapsize-1)<<SHIFT_AMOUNT, (max(goal.x, bounds.x + bounds.w) & INT_MASK) + bounds.w);
    checkRegion.h = min((mapsize-1)<<SHIFT_AMOUNT, (max(goal.y, bounds.y + bounds.h) & INT_MASK) + bounds.h);
    
    increment(&checkRegion, &goal, &intersect, &bounds, true, &r);
    increment(&checkRegion, &goal, &intersect, &bounds, false, &r);

    //vector rv = {bounds.x, bounds.y, ONE_SHIFTED, ONE_SHIFTED};
    //rv.x = bounds.x; rv.y = bounds.y; rv.w = ONE_SHIFTED; rv.h = ONE_SHIFTED;
    r.v = bounds;
    return r;
}