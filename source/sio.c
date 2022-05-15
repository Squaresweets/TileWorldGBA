#include "sio.h"
#include <tonc.h>
#include "main.h"
#include <string.h>

//Much of this comes from here:
//https://github.com/maciel310/gba-mmo/blob/main/source/serial.c
//https://problemkaputt.de/gbatek.htm#sionormalmode

#define SIO_SI BIT(2)
#define SIO_SO BIT(3)
#define SIO_START BIT(7)
#define SIO_CLOCK_INTERNAL BIT(0)
#define SIO_TRANSFER_32 BIT(12)
#define SBB_0 28
SCR_ENTRY *bg_map= se_mem[SBB_0];


//Output stuff

//buffer of 4, hopefully it doesn't back up more than that
//each message is a max of 18 bytes (20 is used as it is divisible by 4)
u8 outbuf[4][20];
u8 numinbuf;
u8 dataoffset;
u8 datalen;

//Input stuff
u32 incomingbuf[6];
u32 expectedlen;
u32 incomingoffset;
u8 previousnibble;

bool startsending = false;
//If we are recieving loads of map data, we want to stream it straight into where it should be, not keep it in a buffer
bool mapdatamode = false;

volatile bool setupmapTrigger = false;


//Length in bytes of each of the things the client could send
u16 datalengthtable[10] = {0, 3, 0, 1, 0, 12, 18, 0, 16, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

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

//=========================== SENDING DATA ===========================
//There may be some more 
void connect()
{
    outbuf[numinbuf][0] = 0x1;
    outbuf[numinbuf][1] = 0x1;
    outbuf[numinbuf][2] = 0x0;
    numinbuf++;
    outbuf[numinbuf][0] = 0x3;
    numinbuf++;
}
void place(u32 x, u32 y, u8 ID)
{
    outbuf[numinbuf][0] = 0x5;
    *(u32*)(&outbuf[numinbuf][1]) = x;
    *(u32*)(&outbuf[numinbuf][5]) = y;
    outbuf[numinbuf][9] = 0x1;
    outbuf[numinbuf][10] = ID;
    numinbuf++;
}
void move(u8 keys)
{
    outbuf[numinbuf][0] = 0x6;
    outbuf[numinbuf][1] = keys;
    *(float*)(&outbuf[numinbuf][1]) = Fixed_to_float(playerx * 32);
    *(float*)(&outbuf[numinbuf][5]) = Fixed_to_float(playery * 32);
    *(float*)(&outbuf[numinbuf][9]) = Fixed_to_float(xv * 32);
    *(float*)(&outbuf[numinbuf][13]) = Fixed_to_float(yv * 32);
    numinbuf++;
}
void requestChunks(s32 xDir, s32 yDir)
{
    outbuf[numinbuf][0] = 0x8;
    *(u32*)(&outbuf[numinbuf][1]) = MapOffsetX;
    *(u32*)(&outbuf[numinbuf][5]) = mapOffsetY;
    *(u32*)(&outbuf[numinbuf][9]) = xDir;
    *(u32*)(&outbuf[numinbuf][13]) = yDir;
    numinbuf++;
}

//=========================== MAP STUFF ===========================
/*
    Tileworld data is orgnaised in 225 16x16 blocks
    0   1   2   3...
    15  16  17  18...
    etc.

    Tilemap displayed is organised in 4 32x32 blocks               
    0 1
    2 3

    To get these to line up I split each 32x32 block in the tilemap into 4 16x16
    0  1    4  5
    2  3    6  7

    8  9    12 13
    10 11   14 15
    (Keep in mind each of these is 16x16 (One chunk))


    So for the following function I should give it a memory address in the map array for the chunk I wanna copy
    And then give me an ID 0-15 for one of the chunks as seen above to copy it into
    

    First, I wanna get given the ID 0-15 in this pattern
    0  1    2  3
    4  5    6  7

    8  9    10 11
    12 13   14 15
    And then convert it to the other pattern
    Now I could do logic for this, but a conversion table is easier
*/
u16 mapIDconversiontable[16] = {0,  1,  4,  5, 
                                2,  3,  6,  7, 

                                8,  9,  12, 13, 
                                10, 11, 14, 15};


//sx and sy = Where to place on the tilemap (0-3)
//x and y = Where to get from map (0-14)
void setChunk(int sx, int sy, int x, int y)
{	
    sx = mod(sx, 4); sy = mod(sy, 4);
    x = mod(x, 15); y = mod(y, 15);
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
    u8 r = 3*(direction == 1);
    mapX += direction;
    for(u8 i=0; i<4; i++)
        setChunk(mapX - 5 + r, i+mapY-5, mapX + r, mapY+i);
}
void loadChunksUD(int direction) //-1 = up, 1 = down
{  
    u8 d = 3*(direction == 1);
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



void handle_serial()
{
    //Fetch our data
    u32 data = REG_SIODATA32;
    REG_SIOCNT |= SION_ENABLE;
    REG_SIODATA32 = 0;

    if (!startsending)
    {
        if(data == 0xDEADBEEF)
            startsending = true;
        return;
    }

    //=========================== OUTGOING DATA ===========================
    //Time to work out what we need to send
    if(numinbuf != 0 && datalen == 0)
    {
        datalen = datalengthtable[outbuf[0][0]];
        REG_SIODATA32 = datalen;
    }
    else if (numinbuf != 0 && dataoffset < datalengthtable[outbuf[0][0]]) //Calculates how long the message should be by the message ID
    {
        REG_SIODATA32 = outbuf[0][dataoffset++] << 24;
        REG_SIODATA32 |= outbuf[0][dataoffset++] << 16;
        REG_SIODATA32 |= outbuf[0][dataoffset++] << 8;
        REG_SIODATA32 |= outbuf[0][dataoffset++];
    }
    //Check if we have reached the end of the current thing to send
    if (numinbuf != 0 && dataoffset >= datalengthtable[outbuf[0][0]])
    {
        numinbuf--;
        dataoffset = 0;
        datalen = 0;
        //Shift array left
        memmove(&outbuf, &outbuf[1], 60);
    }

    //if(cutOffSerialTest) return;
    //=========================== INCOMING DATA ===========================
    /*Incoming is trickier as I am going to get ALOT of data through at once
    Like 60kb worth, not going to be fun
    It would be better not to store this in a buffer
    but instead to put it in the exact place in memory where it is being stored

    We are going to be getting 225 16x16 chunks
    It would be better if it was stored in memory in 32x32 chunks like the GBA uses
    but with the 240x240 map this isn't really possible

    Another problem is that of the GBA tilemap only being 64x64
    To get around this when you get to one side of the map the other side is
    Filled in with the new data

    This is the same on a bigger level for when new map data is added
    */
    if (!expectedlen)
    {
        expectedlen = data;
        if (expectedlen > 57600)
        {
            mapdatamode = true;
            expectedlen /= 2; //Since we are dealing with nibbles
        }
        return;
    }

    if (mapdatamode)
    {
        //The server sends the data type at the start, which shifts everything along by one
        //WHYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
        //If it was just a u16 it would be easy, but no
        //Now I have to do a bunch of really annoying shifting
        //Thanks DFranx

        //JK :D
        if(incomingoffset != 0) //So that we don't include the message ID in the map data
        {
            map[incomingoffset] = ((data >> 24) & 0xF) | (previousnibble << 4);
            incomingoffset +=1;
        }
        if(incomingoffset != 28800)
        {
            map[incomingoffset] = ((data >> 8) & 0xF) + (((data >> 16) & 0xF) << 4);
            incomingoffset += 1;
            previousnibble = (data) & 0xF;
        }

    }
    else
    {
        incomingbuf[incomingoffset] = data;
        incomingoffset += 4;
    }

    if (incomingoffset >= expectedlen)
    {
        //TODO: Do stuff with data
        expectedlen = 0;
        incomingoffset = 0;
        previousnibble = 0;
        if(mapdatamode)
            setupmapTrigger = true;
        mapdatamode = false;
    }
}

//We are never the master, so no need to set any clock stuff
void sioInit()
{
    REG_RCNT = 0;
    REG_SIODATA32 = 0;
    REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;
    irq_add(II_SERIAL, handle_serial);
    connect();
}