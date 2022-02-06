#include "toolbox.h"
#include "input.h"
#include "TileMap.h"
#include "types.h"
#include "memmap.h"		// (tonc_memmap.h)
#include "memdef.h"		// (tonc_memdef.h)

#include <string.h>


#define CBB_0  0
#define SBB_0 28

#define SCREEN_W      240
#define SCREEN_H      160
//Screen offset from player
#define SCREEN_O_W    116.0
#define SCREEN_O_H    76.0

#define CROSS_TX 15
#define CROSS_TY 10

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;


u32 se_index(u32 tx, u32 ty, u32 pitch)
{	
	u32 sbb= ((tx>>5)+(ty>>5)*(pitch>>5));

	return sbb*1024 + ((tx&31)+(ty&31)*32);
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void init_map()
{
	int ii, jj, c;

	// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_64x64;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;
	bg0_pt.x = 0;
	bg0_pt.y = 0;

	// create the tiles: basic tile and a cross
    memcpy(&tile_mem[CBB_0][0], TileMapTiles, TileMapTilesLen);
    memcpy(&tile_mem[4][0], TileMapTiles, TileMapTilesLen);
    memcpy(pal_bg_mem, TileMapPal, TileMapPalLen);
    memcpy(pal_obj_mem, TileMapPal, TileMapPalLen);
    //memcpy(pal_obj_mem, playerspritePal, 1);

	// Create a map: four contingent blocks of 
	//   0x0000, 0x1000, 0x2000, 0x3000.
	SCR_ENTRY *pse= bg0_map;
	for(ii=0; ii<4; ii++)
	{
		for(jj=0; jj<32*32; jj++)
		{
			c = 0;
			if (ii>1) c = 8;
			else if (jj >= 32*31) c = 9;
			//ATM i am just hard coding in the house, obviously this will not stay, but it gives me somewhere to test physics
			if(ii == 0)
			{
				//The roof of the house
				if(jj == 802 || (jj > 832 && jj < 836) || (jj > 863 && jj < 869)) c = 2;
				//The house
				if((jj > 896 && jj < 900) || (jj > 928 && jj < 932) || (jj > 960 && jj < 964)) c = 7;
			}

			*pse++= SE_PALBANK(0) | c;
		}
	}
}

int main()
{
	init_map();

	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ;
    oam_init(obj_buffer, 128);

	float playerx = 0, playery = 0;
    u32 tid= 12, pb= 0;      // (3) tile id, pal-bank
    OBJ_ATTR *player= &obj_buffer[0];

    obj_set_attr(player, 
        ATTR0_SQUARE,              // Square, regular sprite
        ATTR1_SIZE_8,              // 8x8p, 
        ATTR2_PALBANK(pb) | tid);   // palbank 0, tile 0
	while(1)
	{
		vid_vsync();

		key_poll();
		//bg0_pt.x += key_tri_horz();
		//bg0_pt.y += key_tri_vert();
		playerx += key_tri_horz();
		playery += key_tri_vert();
		
		//bg0_pt.x = (lerp(playerx, bg0_pt.x + SCREEN_O_W, 0.01f)) - SCREEN_O_W;
		//bg0_pt.y = (lerp(playery, bg0_pt.y + SCREEN_O_H, 0.01f)) - SCREEN_O_H;
		bg0_pt.x = playerx - SCREEN_O_W;
		bg0_pt.y = playery - SCREEN_O_H;

		//Place player at correct position on the screen
    	obj_set_pos(player, playerx - bg0_pt.x, playery - bg0_pt.y);
        oam_copy(oam_mem, obj_buffer, 1);   // (6) Update OAM (only one now)

		REG_BG_OFS[0]= bg0_pt;	// write new position
	}
	return 0;
}
