#include "util.h"

u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}
s32 map_index(s32 tx, s32 ty)
{
	s32 sbb = ((tx>>4) + (ty>>4)*(15));
	
	return (sbb<<8) + ((tx&15)+((ty&15)<<4));
}
//https://codereview.stackexchange.com/questions/151049/endianness-conversion-in-c
u32 Reverse32(u32 value) 
{
    return (((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) <<  8) |
            ((value & 0x00FF0000) >>  8) |
            ((value & 0xFF000000) >> 24));
}
u32 swapNibbles(u32 x)
{
    return ( (x & 0x0F)<<4 | (x & 0xF0)>>4 );
}
u32 ReverseNibbles32(u32 value) 
{
    return ((swapNibbles(value & 0x000000FF)) |
            (swapNibbles((value & 0x0000FF00) >> 8) <<  8) |
            (swapNibbles((value & 0x00FF0000) >> 16) <<  16) |
            (swapNibbles((value & 0xFF000000) >> 24) << 24));
}