#include "map.h"
#include "sio.h"
#include "main.h"
#include "util.h"
#include "Colly.h"

#include <string.h>
#include <tonc.h>

//Much of this comes from here:
//https://github.com/maciel310/gba-mmo/blob/main/source/serial.c
//https://problemkaputt.de/gbatek.htm#sionormalmode

#define SIO_SI BIT(2)
#define SIO_SO BIT(3)
#define SIO_START BIT(7)
#define SIO_CLOCK_INTERNAL BIT(0)
#define SIO_TRANSFER_32 BIT(12)

//FOR ALL FUTURE REFERENCE!!!
//In this file, output is gba->tileworld
//Input is tileworld->GBA


//~~~Output stuff~~~
//buffer of 4, hopefully it doesn't back up more than that
//each message is a max of 18 bytes (20 is used as it is divisible by 4)
u8 outbuf[4][20];
u8 numinOutBuf;
u8 dataoffset;
u8 datalen;

//~~~Input stuff~~~
u32 incomingdata[6];
u32 expectedlen;
u32 incomingoffset;
u8 previousnibble;

bool startsending = false;



//Length in bytes of each of the things the client could send
u16 datalengthtable[10] = {0, 3, 0, 1, 0, 11, 18, 0, 17, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

//=========================== SENDING DATA ===========================
void connect()
{
    outbuf[numinOutBuf][0] = 0x1;
    outbuf[numinOutBuf][1] = 0x1;
    outbuf[numinOutBuf][2] = 0x0;
    numinOutBuf++;
    outbuf[numinOutBuf][0] = 0x3;
    numinOutBuf++;
}
void place(u32 x, u32 y, u8 ID)
{
    if(numinOutBuf>3) return; //Lets just hope it doesn't back up more than this
    outbuf[numinOutBuf][0] = 0x5;
    *(u32*)(&outbuf[numinOutBuf][1]) = x;
    *(u32*)(&outbuf[numinOutBuf][5]) = y;
    outbuf[numinOutBuf][9] = 0x1;
    outbuf[numinOutBuf][10] = ID;
    numinOutBuf++;
}
void sioMove()
{
    if(numinOutBuf) return; //Only send movement stuff if the buffer is clear, for this reason we don't have to check numinbuf
    
    outbuf[numinOutBuf][0] = 0x6;
    //Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
    outbuf[numinOutBuf][1] = (key_is_down(KEY_UP)) |
                          (key_is_down(KEY_DOWN) << 1) |
                          (key_is_down(KEY_LEFT) << 2) |
                          (key_is_down(KEY_RIGHT) << 3) |
                          (key_is_down(KEY_A) << 4);

    //Have to do a couple of things to position first
    //-0.5 from both to get center of player(not top left)
    //Add 1 to X (for some reason, just how TW likes it)
    //Minus 32 to zero the position
    //Multiply them by 32 (idk why, just how tileworld does it)
    //Finally convert to a float, gotta make it little endian because reasons
    *(float*)(&outbuf[numinOutBuf][2]) = (Fixed_to_float((playerx-(ONE_SHIFTED/2) + ONE_SHIFTED -(INITIAL_PLAYER_POS)) * 32));
    *(float*)(&outbuf[numinOutBuf][6]) = (Fixed_to_float((playery+(ONE_SHIFTED/2)               -(INITIAL_PLAYER_POS)) * 32));

    *(float*)(&outbuf[numinOutBuf][10]) = Fixed_to_float(xv * 64);
    *(float*)(&outbuf[numinOutBuf][14]) = Fixed_to_float(-yv * 32);
    numinOutBuf++;
}
void requestChunks(int xDir, int yDir)
{
    if(numinOutBuf>3) return; //Lets just hope it doesn't back up more than this
    mapOffsetX += xDir; mapOffsetY += yDir;
    outbuf[numinOutBuf][0] = 0x8;
    //*(int*)(&outbuf[numinOutBuf][1]) = (mapOffsetX*16);
    //*(int*)(&outbuf[numinOutBuf][5]) = (mapOffsetY*16);
    *(int*)(&outbuf[numinOutBuf][3]) = Reverse32(0x12345678);
    //*(int*)(&outbuf[numinOutBuf][5]) = 6;
    //*(int*)(&outbuf[numinOutBuf][9]) = xDir;
    //*(int*)(&outbuf[numinOutBuf][13]) = yDir;
    numinOutBuf++;
}

//If we are recieving loads of map data, we want to stream it straight into where it should be, not keep it in a buffer
bool mapdatamode = false;
bool newchunksmode = false;
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
    if(numinOutBuf != 0 && datalen == 0)
    {
        datalen = datalengthtable[outbuf[0][0]];
        REG_SIODATA32 = datalen;
    }
    else if (numinOutBuf != 0 && dataoffset < datalengthtable[outbuf[0][0]]) //Calculates how long the message should be by the message ID
    {
        REG_SIODATA32 = outbuf[0][dataoffset++] << 24;
        REG_SIODATA32 |= outbuf[0][dataoffset++] << 16;
        REG_SIODATA32 |= outbuf[0][dataoffset++] << 8;
        REG_SIODATA32 |= outbuf[0][dataoffset++];
    }
    //Check if we have reached the end of the current thing to send
    if (numinOutBuf != 0 && dataoffset >= datalengthtable[outbuf[0][0]])
    {
        numinOutBuf--;
        dataoffset = 0;
        datalen = 0;
        //Shift array left
        memmove(&outbuf, &outbuf[1], 60);
    }

    //=========================== INCOMING DATA ===========================
    //Looking at incoming spawn data
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
        //This way of doing it is really fragmented and annoying,
        //but no packet types are really the same
        if (expectedlen > 57600)
        {
            mapdatamode = true;
            expectedlen /= 2; //Since we are dealing with nibbles
        }
        else if (expectedlen == 11885)
            newchunksmode = true;
        return;
    }

    //INCOMING SPAWN CHUNKS
    if(mapdatamode)
    {
        //this could 100% be improved, but it works so i'm not touching it
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
    else if (newchunksmode)
    {
        processNewChunkData(data, incomingoffset); //map.c
        incomingoffset += 4;
    }
    else
    {
        incomingdata[incomingoffset/4] = Reverse32(data);
        incomingoffset += 4;
    }

    if (incomingoffset >= expectedlen)
    {
        expectedlen = 0;
        incomingoffset = 0;
        previousnibble = 0;
        if(mapdatamode)
            setupmapTrigger = true;
        else if (!newchunksmode)
            processData();
        mapdatamode = false;
        newchunksmode = false;
    }
}

void processData()
{
    u8* incomingbuf8 = (u8*)incomingdata;
    if(!*incomingbuf8) //No data or data has already been processed
        return;
    if(*incomingbuf8 == 5) //Server Place
    {
        int x = *(int*)(incomingbuf8 + 1);
        int y = *(int*)(incomingbuf8 + 5);
        u8 id = incomingbuf8[10];

        //IMPORTANT
        //I'm aware that an identical function is in map.c, however calling 2 layers of functions from an interrupt
        //Breaks stuff, so i have copied it here

        //For this I can use map_index function I made from the util.c file
        //But we also have the issue of how it is all packed, with nibbles
        int index = map_index(x+112, y+112);
        map[index/2] = ((index % 2) ? (map[index/2] & 0xF0) | id         //Attempting to set the right nibble
                                    : (map[index/2] & 0x0F) | id << 4);

        //Now, since the map array is only used when loading new chunks
        //I also have to set it on the actual map
        if(x+112-(16*mapX) < 64 && x+112-(16*mapX) >= 0 && y+112-(16*mapY) < 64 && y+112-(16*mapY) >= 0) //Tests if the tile is in the screen boundaries (probably a better way to do this)
        se_mem[28][se_index(mod(x+112-(16*5), 64), mod(y+112-(16*5), 64), 64)] = id;
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