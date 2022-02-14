 //https://stackoverflow.com/questions/3437404/min-and-max-in-c
 #define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

 #define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}

struct vector {
    float x;
    float y;
    float w;
    float h;
};

bool Intersects(*vector rect, *vector other, *vector intersection)
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

//We are going to assume steps is always 1
vector Check(*vector bounds, *vector goal)
{
    //Ok here is the pointer trickery that I will probably use:
    //I check all the stuff for x, and then I add one to both of the pointers
    //Next time they are checked for "x" it will give y and same for width and height
    vector intersect;
    
}