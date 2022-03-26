#include "sio.h"
#include <tonc.h>
#include "main.h"

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
    REG_SIODATA32 = 0;
    REG_RCNT = 0;
    REG_SIOCNT = SION_CLK_EXT | SION_ENABLE | SIO_MODE_32BIT | SIO_IRQ;
    irq_add(II_SERIAL, handle_serial);
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

u32 messagelen = 0;
u32 expectedlen = 0;
u8 buffer[512];
void handle_serial()
{
    resetPlayerPos();
    //Fetch our data
    u32 data = REG_SIODATA32;
    //We don't want to send anything back
    REG_SIODATA32 = 75;

    REG_SIOCNT |= SION_ENABLE;

    //Check if this is a packet preceding some data, telling us the length to expect
    //Keyword is "BEEF", the second half tells us the length
    if(expectedlen == 0 && (data & 0xFFFF0000) == 0xBEEF0000)
    {
        expectedlen = data & 0xFFFF;
        messagelen = 0;
        return;
    }

    //Gets all the data and puts it in the buffer
    for(int i = 24; i >= 0; i -= 8)
        buffer[messagelen++] = ((data >> i) & 0xFF);

    if (messagelen >= expectedlen)
    {
        //We are on the final byte of the data
        //Gotta check if we have the terminator byte ("DEADBEEF"), if not, ignore
        if (data != 0xDEADBEEF)
        {
            //Probably incorrect data, ignore it
            expectedlen = 0;
            messagelen = 0;
            return;
        }
        //TODO: Decode data and work out what it is
        //This is just a test for now
        if(buffer[0] == 0x65)
            resetPlayerPos();
        
        expectedlen = 0;
        messagelen = 0;
    }
    else if (data == 0xDEADBEEF)
    {
        //Probably incorrect data, ignore it
        expectedlen = 0;
        messagelen = 0;
        return;
    }
}