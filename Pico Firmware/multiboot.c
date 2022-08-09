#include "multiboot.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"

//Read 4 bytes and write four bytes
int rw4(uint32_t data)
{
    //TODO: This
    return 0;
}
//REFERENCE: http://problemkaputt.de/gbatek-bios-multi-boot-single-game-pak.htm
int multiboot(uint8_t* rom, uint64_t len)
{
    if(len > 0x40000)
    {
        printf("*** MULTIBOOT FILESIZE ERROR ***\n");
        return 0; //Failed
    }
    uint32_t temp;

    printf("Turn on the GBA\n");
    while((temp>>16) != 0x7202)
    {
        //Repeat 15 times, if failed: delay 1/16s and restart
        for(uint32_t i = 0; i < 15; i++)
        {
            temp = rw4(0x6200);
            if((temp>>16) == 0x7202)
                break;
            if(i == 14) sleep_ms(62.5);
        }
    }
    printf("Let's do this thing!\n");
    
    rw4(0x6102);

    //Send Nintendo logo (totally legal, I have the copyright ah yes)
    for(int i = 0; i < 96; i++)
        rw4((uint32_t)(rom[i*2]) + ((uint32_t)(rom[(i*2)+1]) << 8));
    
    rw4(0x6200);
    rw4(0x6202);
    rw4(0x63D1);

    temp = rw4(0x63D1);
    if((temp >> 24) != 0x73)
    {
        printf("Failed handshake! How sad\n");
        return 0;
    }
    printf("Handshake successful!\n");

    sleep_ms(62.5);

    uint32_t crcA = (temp >> 16) & 0xFF;
    uint32_t seed = 0xFFFF00D1 | (crcA << 8);
    crcA = (crcA + 0xF) & 0xFF;

    rw4(0x6400 | crcA);

    len += 0xF;
    len &= ~0xF;

    temp = rw4((len - 0x190) / 4);
    uint32_t crcB = (temp >> 16) % 0xFF;
    uint32_t crcC = 0xC387;

    uint32_t* rom4 = (uint32_t*)rom;
    uint32_t dat;
    uint32_t bit;
    for(int i = 0xC0; i < len; i += 4)
    {
        dat = rom4[i/4];
        temp = dat;

        for(int j = 0; j<32; j++)
        {
            bit = (crcC ^ temp) & 1;
            if(bit == 0)
                crcC = (crcC >> 1) ^ 0;
            else
                crcC = (crcC >> 1) ^ 0xc37b;
            temp >>= 1;
        }
        seed = seed * 0x6F646573 + 1;
        dat = seed ^ dat ^ (0xFE000000 - i) ^ 0x43202F2F;

        rw4(dat & 0xFFFFFFFF);
    }

    temp = 0xFFFF0000 | (crcB << 8) | crcA;

    for(int j = 0; j<32; j++)
    {
        bit = (crcC ^ temp) & 1;
        if(bit == 0)
            crcC = (crcC >> 1) ^ 0;
        else
            crcC = (crcC >> 1) ^ 0xc37b;
        temp >>= 1;
    }

    rw4(0x0065);

    while((rw4(0x0065) >> 16) != 0x0075);

    rw4(0x0066);
    rw4(crcC & 0xFFFF);

    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~DONE!~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    return 1;
}