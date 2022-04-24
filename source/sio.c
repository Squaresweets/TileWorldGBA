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

//We are never the master, so no need to set any clock stuff
void sioInit()
{
    REG_RCNT = 0;
    REG_SIODATA32 = 0;
    REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;
    irq_add(II_SERIAL, handle_serial);

    outbuf[0][0] = 0x1;
    outbuf[0][1] = 0x1;
    outbuf[0][2] = 0x0;
}

//Output stuff

//buffer of 4, hopefully it doesn't back up more than that
//each message is a max of 18 bytes (20 is used as it is divisible by 4)
u8 outbuf[4][20];
u8 numinbuf;
u8 dataoffset;
u8 datalen;

//Length in bytes of each of the things the client could send
u8 datalengthtable[10] = {0, 3, 0, 1, 0, 12, 18, 0, 16, 0}; //Message number 9 is used for message, but atm I am not planning on implementing this

void handle_serial()
{
    //Fetch our data
    u32 data = REG_SIODATA32;
    REG_SIODATA32 = 0;

    //=========================== OUTGOING DATA ===========================
    //Time to work out what we need to send
    if(numinbuf != 0 && datalen == 0)
    {
        datalen = datalengthtable[outbuf[0][3]];
        REG_SIODATA32 = datalen;
    }
    else if (numinbuf != 0 && dataoffset < datalengthtable[outbuf[0][3]]) //Calculates how long the message should be by the message ID
    {
        REG_SIODATA32 = outbuf[0][dataoffset++];
        REG_SIODATA32 |= outbuf[0][dataoffset++];
        REG_SIODATA32 |= outbuf[0][dataoffset++];
        REG_SIODATA32 |= outbuf[0][dataoffset++];
    }
    //Check if we have reached the end of the current thing to send
    if (numinbuf != 0 && dataoffset >= datalengthtable[outbuf[0][3]])
    {
        numinbuf--;
        dataoffset = 0;
        datalen = 0;
        //Shift array left
        memmove(&outbuf, &outbuf[1], 60);
    }
    
    REG_SIOCNT |= SION_ENABLE;
    //=========================== INCOMING DATA ===========================
}