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
    float interright = min(rect->x + 1, other->x + 1);
    float interbottom = min(rect->y + 1, other->y + 1);

    if (interleft < interright && intertop < interbottom)
    {
        intersection->x = interleft; intersection->y = intertop;
        intersection->w = interright-interleft; intersection->h = interbottom-intertop;
        return true;
    }
    return false;
}

void increment(vector *checkRegion, vector *goal, vector *intersect, vector *bounds, bool X)
{
    if(X)
        bounds->x +=  (goal->x - bounds->x);
    else
        bounds->y +=  (goal->y - bounds->y);

    for(int y = (int)checkRegion->y; y <= (int)checkRegion->h; y++)
    {
        for(int x = (int)checkRegion->x; x <= (int)checkRegion->w; x++)
        {
            int id = se_mem[28][se_index(x,y,mapsize)];

            //If it is air carry on
            if(id == 0) continue;

            vector cell;
            cell.x = x; cell.y = y; cell.w = 1; cell.h = 1;
            if(Intersects(&cell, bounds, intersect))
            {
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
vector Check(vector bounds, vector goal)
{
    vector intersect;
    //Ok here is the pointer trickery that I will probably use:
    //I check all the stuff for x, and then I add one to both of the pointers
    //Next time they are checked for "x" it will give y and same for width and height

    vector checkRegion;
    checkRegion.x = max(0, (int)(min(goal.x, bounds.x) - bounds.w));
    checkRegion.y = max(0, (int)(min(goal.y, bounds.y) - bounds.h));
    checkRegion.w = min(mapsize-1, (int)(max(goal.x, bounds.x + bounds.w)) + bounds.w);
    checkRegion.h = min(mapsize-1, (int)(max(goal.y, bounds.y + bounds.h)) + bounds.h);
    
    increment(&checkRegion, &goal, &intersect, &bounds, true);
    increment(&checkRegion, &goal, &intersect, &bounds, false);

    vector r;
    r.x = bounds.x; r.y = bounds.y; r.w = 1; r.h = 1;
    return r;
}