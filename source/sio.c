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

//Output stuff

//buffer of 4, hopefully it doesn't back up more than that
//each message is a max of 18 bytes (20 is used as it is divisible by 4)
u8 outbuf[4][20];
u8 numinbuf;
u8 dataoffset;
u8 datalen;

//Input stuff
u32 incomingbuf[6];
u8 expectedlen;
u8 incomingoffset;

bool startsending = false;

//Length in bytes of each of the things the client could send
u8 datalengthtable[10] = {0, 3, 0, 1, 0, 12, 18, 0, 16, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

//We are never the master, so no need to set any clock stuff
void sioInit()
{
    REG_RCNT = 0;
    REG_SIODATA32 = 0;
    REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;
    irq_add(II_SERIAL, handle_serial);

    connect();
}

void connect()
{
    outbuf[numinbuf][0] = 0x1;
    outbuf[numinbuf][1] = 0x1;
    outbuf[numinbuf][2] = 0x0;
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


}