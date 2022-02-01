#include "toolbox.h"
#include "input.h"
#include "TileMap.h"

#include <string.h>


#define CBB_0  0
#define SBB_0 28

#define CROSS_TX 15
#define CROSS_TY 10

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];


u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}

void init_map()
{
	int ii, jj;

	// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_64x64;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;

	// create the tiles: basic tile and a cross
    memcpy(&tile_mem[CBB_0][0], TileMapTiles, TileMapTilesLen);
    memcpy(pal_bg_mem, TileMapPal, TileMapPalLen);

	// Create a map: four contingent blocks of 
	//   0x0000, 0x1000, 0x2000, 0x3000.
	SCR_ENTRY *pse= bg0_map;
	for(ii=0; ii<4; ii++)
		for(jj=0; jj<32*32; jj++)
			*pse++= SE_PALBANK(0) | jj%32;
}

int main()
{
	init_map();
	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ;

	u32 tx, ty, se_curr, se_prev= CROSS_TY*32+CROSS_TX;
	
	bg0_map[se_prev]++;	// initial position of cross
	while(1)
	{
		vid_vsync();

		key_poll();
		bg0_pt.x += key_tri_horz();
		bg0_pt.y += key_tri_vert();

		// Testing bg_se_id()
		// If all goes well the cross should be around the center of
		// the screen at all times.
		tx= ((bg0_pt.x>>3)+CROSS_TX) & 0x3F;
		ty= ((bg0_pt.y>>3)+CROSS_TY) & 0x3F;
		
		se_curr= se_index(tx, ty, 64);
		if(se_curr != se_prev)
		{
			bg0_map[se_prev]--;
			bg0_map[se_curr]++;
			se_prev= se_curr;
		}

		REG_BG_OFS[0]= bg0_pt;	// write new position
	}
	return 0;
}
