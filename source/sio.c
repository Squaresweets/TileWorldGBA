#include "sio.h"

//https://pastebin.com/a6h3RMd5
//https://problemkaputt.de/gbatek.htm#sionormalmode

//We are never the master, so no need to set any clock stuff
void sioInit()
{
    //Initialise the REG_RCNT register
    REG_RCNT = 0;
    //Set the transfer length to 32 bit
    REG_SIOCNT = SIO_TRANSFER_32;
}
/*
u32 handle_serial()
{
    // - Initialize data which is to be sent to master
    REG_SIODATA32 = data;
    // - Set Start=0 and SO=0 (SO=LOW indicates that slave is (almost) ready).
    BIT_CLEAR(REG_SIOCNT, SIO_START | SIO_SO);
    // - Set Start=1 and SO=1 (SO=HIGH indicates not ready, applied after transfer).
    //   (Expl. Old SO=LOW kept output until 1st clock bit received).
    //   (Expl. New SO=HIGH is automatically output at transfer completion).
    BIT_SET(REG_SIOCNT, SIO_START | SIO_SO);
    // - Set SO to LOW to indicate that master may start now.
    BIT_CLEAR(REG_SIOCNT, SIO_SO);
    // - Wait for IRQ (or for Start bit to become zero). (Check timeout here!)
    while (REG_SIOCNT & SIO_START);
    // - Process received data.
    return REG_SIODATA32;
}*/

u32 handle_serial(u32 data)
{
    // - Initialize data which is to be sent to master
    REG_SIODATA32 = data;
    // - Set Start=0 and SO=0 (SO=LOW indicates that slave is (almost) ready).
    BIT_CLEAR(REG_SIOCNT, SIO_START | SIO_SO);
    // - Set Start=1 and SO=1 (SO=HIGH indicates not ready, applied after transfer).
    //   (Expl. Old SO=LOW kept output until 1st clock bit received).
    //   (Expl. New SO=HIGH is automatically output at transfer completion).
    BIT_SET(REG_SIOCNT, SIO_START | SIO_SO);
    // - Set SO to LOW to indicate that master may start now.
    BIT_CLEAR(REG_SIOCNT, SIO_SO);
    // - Wait for IRQ (or for Start bit to become zero). (Check timeout here!)
    while (REG_SIOCNT & SIO_START);
    // - Process received data.
    return REG_SIODATA32;
}