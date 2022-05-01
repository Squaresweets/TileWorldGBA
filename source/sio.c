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


//Length in bytes of each of the things the client could send
u16 datalengthtable[10] = {0, 3, 0, 1, 0, 12, 18, 0, 16, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

//Where the map is stored in memory [outer y][outer x][inner y][inner x]
u8 map[28800];


//=========================== SENDING DATA ===========================
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
void move(u8 keys, u32 x, u32 y, u32 xv, u32 yv)
{
    outbuf[numinbuf][0] = 0x6;
    outbuf[numinbuf][1] = keys;
    //TODO: I gotta do other stuff first, bare with
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
u16 mapIDconversiontable[16] = {0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15};
void setChunk(u8 ID, u8 x, u8 y)
{	
    SCR_ENTRY *pse= bg_map;
    //First get our pointer to the start of the chunk we wanna edit
    ID = mapIDconversiontable[ID];
    ID = (ID/2)*512 + (ID%2)*16;
	*pse += ID;
    u8 ii = 0;
    int c = 0;
    for(u8 i=0;i<16;i++)
    {
        for(u8 j=0;j<16;j++)
        {
            c = map[x + (y*15) + (ii)/2];
            if(c % 2 == 0)
                c = c>>4;
            else
                c &= 0xFF;
            *pse++ = SE_PALBANK(0) | c;
            ii++;
        }
        *pse += 16;
    }
}
void setupMap()
{
    //Starting map at position (5,5)
    for(u8 y=0;y<4;y++)
    {
        for(u8 x=0;x<4;x++)
            setChunk(x+(4*y), x+5, y+5);
    }
}

//If we are recieving loads of map data, we want to stream it straight into where it should be, not keep it in a buffer
bool mapdatamode = false;

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

    Warning: May countain spaghetti code
    */
    if (!expectedlen)
    {
        expectedlen = data;
        if(expectedlen == 57601) //We are recieving map data
            mapdatamode = true;
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
            map[incomingoffset] = ((data >> 24) & 0xFF) | previousnibble;
            incomingoffset +=2;
        }
        map[incomingoffset] = (data >> 8) & 0xFFFF;
        incomingoffset += 2;
        previousnibble = (data) & 0xFF;
    }
    else
    {
        incomingbuf[incomingoffset] = data;
        incomingoffset += 4;
    }

    if (incomingoffset >= expectedlen)
    {
        //TODO: Do stuff with data
        if(mapdatamode)
            setupMap();
        expectedlen = 0;
        incomingoffset = 0;
        mapdatamode = false;
        previousnibble = 0;
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