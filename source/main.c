#include "toolbox.h"
#include "input.h"
#include "TileMap.h"
#include "types.h"
#include "memmap.h"
#include "memdef.h"

#include "Colly.h"

#include <stdbool.h>
#include <string.h>
#include <math.h>


#define CBB_0  0
#define SBB_0 28

#define SCREEN_W      240
#define SCREEN_H      160
//Screen offset from player
#define SCREEN_O_W    116
#define SCREEN_O_H    76

#define CROSS_TX 15
#define CROSS_TY 10

//Fixed point stuff https://stackoverflow.com/questions/10067510/fixed-point-arithmetic-in-c-programming
#define SHIFT_AMOUNT 16
#define ONE_SHIFTED (1 << SHIFT_AMOUNT)
#define SHIFT_MASK ((1 << SHIFT_AMOUNT) - 1)

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;


//Fixed point, with a shift value of 16
int playerx = 10 << SHIFT_AMOUNT, playery = 30 << SHIFT_AMOUNT;
int camerax = 0, cameray = 30 << SHIFT_AMOUNT;
int xv = 0, yv = 0;
OBJ_ATTR *player= &obj_buffer[0];

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

			*pse++ = SE_PALBANK(0) | c;
		}
	}
}

void movement()
{
	vector bounds = {playerx, playery, ONE_SHIFTED, ONE_SHIFTED};
	
	vector t = {playerx, playery + 6, ONE_SHIFTED, ONE_SHIFTED};
	bool grounded = Check(bounds, t).collided;

	xv += key_tri_horz() * (ONE_SHIFTED / 32);

	if(key_tri_fire() > 0 && grounded)
		yv += ((25<<SHIFT_AMOUNT) / 32);
	else
		yv -= 88166 / 32; //(8166 = 1.3453 << 16)
	
	//Apply friction
	yv *= 0.951;
	xv *= 0.915;

	vector g = {playerx + xv, playery - yv, ONE_SHIFTED, ONE_SHIFTED};
	vector v = Check(bounds, g).v;
	playerx = v.x; playery = v.y;


	//Stuff for next frame
	g.y = 0;
	xv *= !Check(bounds, g).collided;
	g.y = playery - yv; g.x = 0;
	yv *= !Check(bounds, g).collided;


	camerax += (playerx - camerax) * 0.05f;
	cameray += (playery - cameray) * 0.05f;
}
void renderPlayer()
{
	//Rendering player to screen
	//The -3 stuff is confusing, but basically just divides everything by the fixed point stuff to get the actual amount
	//And then times it by 8 (<<3) to get it how the gameboy likes it
	bg0_pt.x = (camerax>>(SHIFT_AMOUNT-3)) - SCREEN_O_W;
	bg0_pt.y = (cameray>>(SHIFT_AMOUNT-3)) - SCREEN_O_H;
	//Place player at correct position on the screen
    obj_set_pos(player, (playerx - (bg0_pt.x<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3),
						(playery - (bg0_pt.y<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3));
    oam_copy(oam_mem, obj_buffer, 1);   // (6) Update OAM (only one now)

	REG_BG_OFS[0]= bg0_pt;	// write new position
}

int main()
{
	init_map();

	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ;
    oam_init(obj_buffer, 128);

    u32 tid= 12, pb= 0;      // (3) tile id, pal-bank

    obj_set_attr(player, 
        ATTR0_SQUARE,              // Square, regular sprite
        ATTR1_SIZE_8,              // 8x8p, 
        ATTR2_PALBANK(pb) | tid);   // palbank 0, tile 0
	while(1)
	{
		vid_vsync();
		key_poll();

		movement();
		renderPlayer();
	}
	return 0;
}
