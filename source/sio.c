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
u32 incomingdata[128];
u8 numinInBuf;
//This is pretty weird to have two, but it helps with keeping everything threadsafe
//More information in the handle serial function
u32 secondaryincomingdata[128];
u8 numinsecondInBuf;

bool currentBuffer = 0; //0=incomingdata, 1=secondaryincomingdata

u32 incomingpacket[6]; //Packets of data, in the correct order
u32 expectedlen;
u32 incomingoffset;

bool startsending = false;

//Length in bytes of each of the things the client could send
u8 datalengthtable[10] = {0, 3, 0, 1, 0, 12, 18, 0, 17, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

//=========================== SENDING DATA ===========================
//Note to self, this could have potentialy been done with unions and structs
void connect()
{
    outbuf[numinOutBuf][0] = 0x1;
    outbuf[numinOutBuf][1] = 0x1;
    outbuf[numinOutBuf][2] = 0x0;
    numinOutBuf++;
    ping();
}
void ping()
{
    if(numinOutBuf>3) return; //Lets just hope it doesn't back up more than this
    outbuf[numinOutBuf][0] = 0x3;
    numinOutBuf++;
    pingtimer = 0;
}
void place(s32 x, s32 y, u8 ID)
{
    setTile(x,y,ID);
    if(numinOutBuf>3) return; //Lets just hope it doesn't back up more than this

    outbuf[numinOutBuf][0] = 0x5;
    memcpy(&outbuf[numinOutBuf][1], &x, 4); //memcpy used due to allignment errors
    memcpy(&outbuf[numinOutBuf][5], &y, 4);
    outbuf[numinOutBuf][9] = 0x1;
    outbuf[numinOutBuf][10] = ID;
    numinOutBuf++;
}
void sioMove()
{
    if(numinOutBuf) return; //Only send movement stuff if the buffer is clear, for this reason we don't have to check numinbuf
    
    outbuf[numinOutBuf][0] = 0x6;
    //Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
    if(!placeMode) //If we are in placeMode ignore inputs
        outbuf[numinOutBuf][1] = (key_is_down(KEY_UP))         |
                                 (key_is_down(KEY_DOWN)  << 1) |
                                 (key_is_down(KEY_LEFT)  << 2) |
                                 (key_is_down(KEY_RIGHT) << 3) |
                                 (key_is_down(KEY_A)     << 4);

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
    memcpy(&outbuf[numinOutBuf][1], &mapOffsetX, 4); //memcpy used due to allignment errors
    memcpy(&outbuf[numinOutBuf][5], &mapOffsetY, 4);
    memcpy(&outbuf[numinOutBuf][9], &xDir, 4);
    memcpy(&outbuf[numinOutBuf][13], &yDir, 4);
    numinOutBuf++;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~YOU ARE NOW ENTERING INTERRUPT TERRATORY, AS LITTLE CODE AS POSSIBLE HERE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void sio_interrupt()
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
    else if (numinOutBuf != 0 && dataoffset >= datalengthtable[outbuf[0][0]])
    {
        numinOutBuf--;
        dataoffset = 0;
        datalen = 0;
        //Shift array left
        memmove(&outbuf, &outbuf[1], 60);
    }
    //=========================== INCOMING DATA ===========================
    if(!currentBuffer)
    {
        incomingdata[numinInBuf] = data; //The reasoning behind putting this in a buffer is to reduce the length of the interrupt
        numinInBuf++;
        if(numinInBuf > 128) playerx = 0;
    }
    else
    {
        secondaryincomingdata[numinsecondInBuf] = data; //The reasoning behind putting this in a buffer is to reduce the length of the interrupt
        numinsecondInBuf++;
        if(numinsecondInBuf > 128) playerx = 0;
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void handle_serial()
{
    u32 data;
    /*
    RIGHT! time to explain why I need two buffers
    If i just had one buffer, throughout this function if we get more data through it could (and will)
    break stuff, since we don't know on which line it will start the interupt

    So instead we have alternating buffers, when I start processing one I switch it so all
    new data is put into the other, then next time I switch it again.
    This alternating pattern keeps new data from interfering
    tdlr: Make it threadsafe
    */
    currentBuffer = !currentBuffer; //Switch which buffer is being used
    u8 n = currentBuffer ? numinInBuf : numinsecondInBuf;
    for(u8 i=0; i<n; i++)
    {
        if(currentBuffer) data = incomingdata[i]; else data = secondaryincomingdata[i];
        if(!expectedlen) //If the current packet length is 0 (no current packet)
        {
            expectedlen = data; //If data is still 0, this will just repeat
            continue;
        }

        if(expectedlen == 11885) //New chunk data (not spawn)
            processNewChunkData(data, incomingoffset); //This is more complicated, so is in map.c instead
        else if(expectedlen == 57601) //We are dealing with spawn data
        {
            u32 o;
            data = Reverse32(data);
            u8 *d = (u8*)&data;
            for(u8 j=0;j<4;j++)
            {
                o = incomingoffset+j;
                if(!o) continue; //Ignore the first byte
                map[(o-1)/2] += ((o-1)&1) ? (d[j] & 0xF)
                                          : (d[j] << 4); //Put it in the right nibble in the map array
            }
        }
        else if(expectedlen < 25)
            incomingpacket[incomingoffset/4] = Reverse32(data); //Any other packet (except messages)
        incomingoffset += 4;

        if(incomingoffset >= expectedlen)
        {
            if(expectedlen == 57601) setupMap();
            else if(expectedlen<25) processData();
            expectedlen = 0; //Go back to current packet size being 0
            incomingoffset = 0;
        }
    }
    if(currentBuffer) numinInBuf = 0; else numinsecondInBuf = 0; //Clear buffer
}
void processData()
{
    u8* incomingpacket8 = (u8*)incomingpacket;
    if(!*incomingpacket8) //No data or data has already been processed
        return;
    if(*incomingpacket8 == 5) //Server Place
    {
        int x = *(int*)(incomingpacket8 + 1);
        int y = *(int*)(incomingpacket8 + 5);
        u8 id = incomingpacket8[10];
        setTile(x, y, id); //map.c
    }
}

//We are never the master, so no need to set any clock stuff
void sioInit()
{
    REG_RCNT = 0;
    REG_SIODATA32 = 0;
    REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;
    irq_add(II_SERIAL, sio_interrupt);
    connect();
}