#include "Colly.h"
#include "util.h"

#define mapsize 64

int mod(int x,int N){
    return (x % N + N) %N;
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
            int id = se_mem[28][se_index((x >> SHIFT_AMOUNT) & 63, (y >> SHIFT_AMOUNT) & 63,mapsize)];

            //If it is air carry on
            if(id == 0) continue;

            vector cell;
            cell.x = x; cell.y = y; cell.w = ONE_SHIFTED; cell.h = ONE_SHIFTED;
            if(Intersects(&cell, bounds, intersect))
            {
                if(id == 11 || id == 15) { r->ladder = true; continue;} //15 is the debug tile, 11 is ladder
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
    vector intersect = {0,0,0,0};
    checkreturn r = {intersect, false, false};

    vector checkRegion;
    checkRegion.x = (min(goal.x, bounds.x) - bounds.w) & INT_MASK;
    checkRegion.y = (min(goal.y, bounds.y) - bounds.h) & INT_MASK;
    checkRegion.w = (max(goal.x, bounds.x + bounds.w) & INT_MASK) + bounds.w;
    checkRegion.h = (max(goal.y, bounds.y + bounds.h) & INT_MASK) + bounds.h;
    
    increment(&checkRegion, &goal, &intersect, &bounds, true, &r);
    increment(&checkRegion, &goal, &intersect, &bounds, false, &r);

    //vector rv = {bounds.x, bounds.y, ONE_SHIFTED, ONE_SHIFTED};
    //rv.x = bounds.x; rv.y = bounds.y; rv.w = ONE_SHIFTED; rv.h = ONE_SHIFTED;
    r.v = bounds;
    return r;
}