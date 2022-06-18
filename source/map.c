#include "map.h"
#include <tonc.h>
#include "main.h"
#include "colly.h"
#include "util.h"

#define SBB_0 28
SCR_ENTRY *bg_map= se_mem[SBB_0];
//Where the map is stored in memory [outer y][outer x][inner y][inner x]
u8 map[28800];

//The following x y positions represent where the GBA tilemap is on the Map
//Initial values are 5
int mapX = 5, mapY = 5;
//Say we wanted to move the map to the left, first we would subtract 5
//from both values, then if we are moving to the left we +1, right -1
//these values can then be put through mod(4) to show where to place the new chunks we
//just loaded, hopefully that makes sense

//The following x y positions represent the offset of the map from spawn
int MapOffsetX = 0, mapOffsetY = 0;

volatile bool setupmapTrigger = false;

//See howMapIsStored.md for more info on this
u16 mapIDconversiontable[16] = {0,  1,  4,  5, 
                                2,  3,  6,  7, 

                                8,  9,  12, 13, 
                                10, 11, 14, 15};


//sx and sy = Where to place on the tilemap (0-3)
//x and y = Where to get from map (0-14)
void setChunk(int sx, int sy, int x, int y)
{	
    //Looking back on this I could have used se_index to do this
    //But I'm not touching this now it works lol
    sx = mod(sx, 4); sy = mod(sy, 4); //loop it round
    x = mod(x, 15); y = mod(y, 15); //loop it round
    SCR_ENTRY *pse= bg_map;
    //First get our pointer to the start of the chunk we wanna edit
    int ID = mapIDconversiontable[sx+(4*sy)];
    ID = (ID/2)*512 + (ID%2)*16; //Map starters are dealed with in pairs, (Ax16)(Bx16)(Ax16)etc. (Imagine they are interlaced)
	pse += ID;
    
    int ii = 0;
    int c = 0;
    for(u8 i=0;i<16;i++)
    {
        for(u8 j=0;j<8;j++)
        {
            c = map[(x*128) + (y*15*128) + ii];
            *pse++ = SE_PALBANK(0) | ((c>>4) & 0xF);
            *pse++ = SE_PALBANK(0) | (c & 0xF);
            ii++;
        }
        pse += 16;
    }
}

void setupMap()
{
    //Starting map at position (5,5)
    for(u8 y=0;y<4;y++)
    {
        for(u8 x=0;x<4;x++)
        {
            setChunk(x, y, x+5, y+5);
        }
    }
    startMovement = true;
}

void loadChunksLR(int direction) //-1 = left, 1 = right
{
    u8 r = 3*(direction == 1); //Go to other side if we are going to the right
    mapX += direction;
    for(u8 i=0; i<4; i++)
        setChunk(mapX - 5 + r, i+mapY-5, mapX + r, mapY+i);
}
void loadChunksUD(int direction) //-1 = up, 1 = down
{  
    u8 d = 3*(direction == 1); //Go to other side if we are going down
    mapY += direction;
    for(u8 i=0; i<4; i++)
        setChunk(i+mapX-5, mapY - 5 + d, mapX + i, mapY + d);
}

void loadChunks()
{
    if(setupmapTrigger)
    {
        setupMap();
        setupmapTrigger = false;
        return;
    }

    if(!startMovement) return;
    if(camerax < (((mapX-4)*16) << SHIFT_AMOUNT)) loadChunksLR(-1);
    if(camerax > (((mapX-2)*16) << SHIFT_AMOUNT)) loadChunksLR(1);
    if(cameray < (((mapY-4)*16) << SHIFT_AMOUNT)) loadChunksUD(-1);
    if(cameray > (((mapY-2)*16) << SHIFT_AMOUNT)) loadChunksUD(1);
}

/*
The plan for sending and recieving chunks!
In loadChunks check if playerpos is too far in any direction
Places where next chunk should be loaded are: x<64 x>160 y<64 y>160
Then like in normal load chunks we request chunks for that direction using the function in sio.c

When the chunks are recieved it is harder, the question if I should store it in a large buffer like
in map[], however i don't really want to have to do that
Instead I will just create a 1 chunk buffer, with the position where it should be
and one chunk worth of tile data packed into nibbles
Then after I have completed one buffer, I can send it to ANOTHER buffer (just cause I don't want to do
too much on the different thread cause that causes me nightmares) and start on the next one.
Then I can stream that chunk into it's position mod 15 (240/16) and that should put it in the correct place
so that when the next chunk to the left is attempted to be loaded it instead loads the new c hunk

The main problem that this method gives is a whole bunch of shifting and annoying code, so i'll have to 
look into it
*/