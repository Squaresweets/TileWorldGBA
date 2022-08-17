#include "map.h"
#include <tonc.h>
#include "main.h"
#include "colly.h"
#include "util.h"
#include "sio.h"

#include <string.h>

#define SBB_0 28
SCR_ENTRY *bg_map= se_mem[SBB_0];
//Where the map is stored in memory [outer y][outer x][inner y][inner x]
u8* map; //Of length 28800

//The following x y positions represent where the GBA tilemap is on the Map
//Initial values are 5
int mapX = 5, mapY = 5;
//Say we wanted to move the map to the left, first we would subtract 5
//from both values, then if we are moving to the left we +1, right -1
//these values can then be put through mod(4) to show where to place the new chunks we
//just loaded, hopefully that makes sense

//The following x y positions represent the offset of the map from spawn
int mapOffsetX = 0, mapOffsetY = 0;

int ChunkX = 0, ChunkY = 0;

u8 *d; u8 *cx; u8 *cy; //d = data
u8 *c; //c = chunk position
u32 o;

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
    sx &= 3; sy &= 3; //loop it round
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
            *pse++ = ((c>>4) & 0xF);
            *pse++ = (c & 0xF);
            ii++;
        }
        pse += 16;
    }
}

void setTile(u32 x, u32 y, u8 id)
{
    //For this I can use map_index function I made from the util.c file
    //But we also have the issue of how it is all packed, with nibbles
    int index = map_index(x+112, y+112);
    map[index/2] = ((index % 2) ? (map[index/2] & 0xF0) | id         //Attempting to set the right nibble
                                : (map[index/2] & 0x0F) | id << 4);

    //Now, since the map array is only used when loading new chunks
    //I also have to set it on the actual map
    if(x+112-(16*mapX) < 64 && x+112-(16*mapX) >= 0 && y+112-(16*mapY) < 64 && y+112-(16*mapY) >= 0)
        se_mem[28][se_index((x+32) & 63, (y+32) & 63, 64)] = id;
}
void setupMap()
{
    //Starting map at position (5,5)
    for(u8 y=0;y<4;y++)
    {
        for(u8 x=0;x<4;x++)
            setChunk(x, y, x+5, y+5);
    }
    cx = (u8*)&ChunkX; cy = (u8*)&ChunkY; //Setting stuff up for loading new chunks
    startMovement = true;
}
void loadChunksLR(int direction) //-1 = left, 1 = right
{
    //Hey all, Future Matthew here, why did I do it like this?
    u8 r = 3*(direction == 1); //Go to other side if we are going to the right
    mapX += direction;
    for(u8 y=0; y<4; y++)
        setChunk(mapX - 5 + r, y+mapY-5, mapX + r, mapY+y);
}
void loadChunksUD(int direction) //-1 = up, 1 = down
{  
    u8 d = 3*(direction == 1); //Go to other side if we are going down
    mapY += direction;
    for(u8 x=0; x<4; x++)
        setChunk(x+mapX-5, mapY - 5 + d, mapX + x, mapY + d);
}

void loadChunks()
{
    if(placeMode) return;
    //LOADING CHUNKS IN TERMS OF GBA TILEMAP
    if(camerax < (((mapX-4)*16) << SHIFT_AMOUNT)) loadChunksLR(-1);
    if(camerax > (((mapX-2)*16) << SHIFT_AMOUNT)) loadChunksLR(1);
    if(cameray < (((mapY-4)*16) << SHIFT_AMOUNT)) loadChunksUD(-1);
    if(cameray > (((mapY-2)*16) << SHIFT_AMOUNT)) loadChunksUD(1);

    //LOADING NEW CHUNKS
    if((playerx - INITIAL_PLAYER_POS) < ((-32 + (mapOffsetX * 16)) << SHIFT_AMOUNT)) requestChunks(-3, 0);
    if((playerx - INITIAL_PLAYER_POS) > ((32  + (mapOffsetX * 16)) << SHIFT_AMOUNT)) requestChunks(3, 0);
    if((playery - INITIAL_PLAYER_POS) < ((-32 + (mapOffsetY * 16)) << SHIFT_AMOUNT)) requestChunks(0, -3);
    if((playery - INITIAL_PLAYER_POS) > ((32  + (mapOffsetY * 16)) << SHIFT_AMOUNT)) requestChunks(0, 3);
}

/*
The plan for sending and recieving chunks!
In loadChunks check if playerpos is too far in any direction
Places where next chunk should be loaded are: x<64 x>160 y<64 y>160
Then like in normal load chunks we request chunks for that direction using the function in sio.c

When the chunks are recieved it is harder, the question is if I should store it in a large buffer like
in map[], however i don't really want to have to do that
Instead I will just create a 1 chunk buffer, with the position where it should be
and one chunk worth of tile data packed into nibbles
Then after I have completed one buffer, I can send it to ANOTHER buffer (just cause I don't want to do
too much on the different thread cause that causes me nightmares) and start on the next one.
Then I can stream that chunk into it's position mod 15 (240/16) and that should put it in the correct place
so that when the next chunk to the left is attempted to be loaded it instead loads the new c hunk

The main problem that this method gives is a whole bunch of shifting and annoying code, so i'll have to 
look into it

##(Server) 08 (Chunks)
The server sends this when the client requests chunks. Seems to always consist
of 45 chunks (a 3x15 or 15x3 area). Chunks are 16x16.

	0x08
	Chunk amount (uint32 little endian)
	Chunk data * Chunk amount
	(chunk format:
		ChunkX (int32 little endian)
		ChunkY (int32 little endian)
		Tile*256 (uint8)
	)
*/

void processNewChunkData(u32 data, u32 offset)
{
    //In this function >>1 is used instead of /2 since it is more efficient (i think)
    u8 *d = (u8*)&data;
    o = mod((offset-5), 264);
    for(u8 i=0;i<4;i++, o++) //Iterate through each byte in the data seperatly
    {
        if(offset+i < 5) continue; //The first 5 bytes (packet type and amount of chunks) we can ignore
        u8 b = d[3-i]; //Current byte we are on (reversing bytes)
        o -= (o>=264)*264; //Alternate to mod, much less expensive

        //Since o is much more likely to be more than 8 we check that first :D
        if(o >= 8)
        {
            if(o&1) *(c + ((o-8)>>1)) |= b;
            else    *(c + ((o-8)>>1))  = b << 4;
            if (o == 263 && ChunkX+7 >= mapX && ChunkX+7 <= mapX+3 && ChunkY+7 >= mapY && ChunkY+7 <= mapY+3) //We have syncing issues :/
                setChunk(ChunkX+2, ChunkY+2, ChunkX+7, ChunkY+7); //This is a short version of (for X) (ChunkX+5)-mapx+mapX-5
        }
        else if(o < 4)
            cx[o] = b; //Set x
        else if(o < 8)
        {
            cy[o-4] = b; //Set y
            if(o == 7)
                c = &map[128*mod((ChunkX + 7), 15) +
                        1920*mod((ChunkY + 7), 15)]; //Get position in memory of where the current chunk should be put
        }
    }
}



//*********************************** MINI MAP ***********************************
/*
The plan for the minimap whoop whoop
The main map is on SBB 28, I am using SBB 24 as my minimap screenblock

When mini map mode (say that 20 times!) is selected we first set up the minimap tilemap and then change the screenblock
The actual tilemap is pretty simple, just 900 tiles arranged such that it is 240x240 pixels. This is at 0,0 and
so the camera must be changed to 0,0.
The IDs of these tiles are from 16 upward, and are adjusted depending on mapX and mapY.
The actual tiles themselves are changed, via copying from the map array
*/
void EnableMiniMapMode()
{
    u32* p = (u32*)&tile_mem[0][0];
    u32* m = (u32*)map;
    u32 x,y,ii,i,j = 0;
    //In total it should loop for 900 tiles
    //The problem I am facing is that the map array stores stuff in chunks of 16x16, I need it in chunks of 8x8
    for(y = 0; y < 30; y++)
    {
        for(x = 0; x < 30; x++)
        {
            //for each tile set the right tile ID, offset by mapOffset
            se_mem[24][se_index(x, y, 64)] = ((mod(y + (mapOffsetY*2), 30)*30)+
                                               mod(x + (mapOffsetX*2), 30))+16;

            i = (((y*30)+x)+16)*8;
            j = map_index(x*8, y*8)/8;
            for(ii=0; ii<8; ii++)
                p[i + ii] = ReverseNibbles32(m[j + 2*ii]);
        }
        //Sure it is annoying, but every so often we gotta look at the serial stuff to prevent an overflow
        if((y&3) == 0) handle_serial();
    }
}