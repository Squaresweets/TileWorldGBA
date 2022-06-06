#include "util.h"

u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}
u32 map_index(u32 tx, u32 ty)
{
	u32 sbb = ((tx/16) + (ty/16)*(240/16));
	
	return sbb*256 + ((tx%16)+(ty%16)*16);
}