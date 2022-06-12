#include "main.h"
#include "Colly.h"
#include "sio.h"
#include "TileMap.h"
#include "util.h"

#include <string.h>
#include <math.h>
#include <tonc.h>


#define CBB_0  0
#define SBB_0 28

#define SCREEN_W      240
#define SCREEN_H      160
//Screen offset from player
#define SCREEN_O_W    116
#define SCREEN_O_H    76

#define CROSS_TX 15
#define CROSS_TY 10

BG_POINT bg0_pt= { 0, 0 };
SCR_ENTRY *bg0_map= se_mem[SBB_0];

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer= (OBJ_AFFINE*)obj_buffer;


//Fixed point, with a shift value of 16
int playerx = 32 << SHIFT_AMOUNT, playery = 32 << SHIFT_AMOUNT;
int camerax = 32 << SHIFT_AMOUNT, cameray = 32 << SHIFT_AMOUNT;
int xv = 0, yv = 0;
bool startMovement = false;
OBJ_ATTR *player= &obj_buffer[0];

void init_map()
{

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

	//Code to load loading screen
	int ii, jj, c;
	SCR_ENTRY *pse= bg0_map;
	for(ii=0; ii<4; ii++)
	{
		for(jj=0; jj<32*32; jj++)
		{
			//Here we are hardcoding the loading screen
			//I KNOW this code is ugly, but in terms of saving storage
			//This makes the most sense, since storing the entire loading screen
			//As a tilemap would take up loads of data which would have to be sent over

			//God I hate this code, I really do, if you are reading this please forgive me
			//This is not who I really am, this goddam loading screen haunted my nightmares
			c = 0;
			//LOADING
			if((ii == 0) && (jj>=787) && (jj<896) && (jj%32 > 18) &&(LOAD[(jj-768)/32][(jj%32)-19])) c = 5;
			if((ii == 1) && (jj>=768) && (jj<878) && (jj%32 < 14) &&(DING[(jj-768)/32][(jj%32)])) c = 5;
			if(ii == 1 && ((jj==938) || (jj>968 && jj < 972) || (jj>999 && jj < 1005))) c = 2; //Roof of house
			else if(ii == 2)
			{
				if(jj > 224) c = 8; //Dirt at bottom
				else if(jj>=128 && (jj%32)<21) c = 8; //Bunch of dirt on left
				else if((jj > 189 && jj < 193) || (jj > 218 && jj < 225)) c = 8; //Dirt on right
				else if((jj>96 && jj < 117) || (jj>157 && jj < 160) || (jj>186 && jj < 190) || (jj>212 && jj < 219)) c = 9; //Grass on left and right
			}
			else if(ii == 3)
			{
				if(jj>127) c = 8; //Dirt
				if((jj>127 && jj < 134) || (jj>101 && jj<128)) c = 9; //Grass
				else if(jj<96 && ((jj%32)>8) && ((jj%32)<12)) c = 7; //Main section of house
			}
			*pse++ = SE_PALBANK(0) | c;
		}
	}
}

void movement()
{
	vector bounds = {playerx, playery, ONE_SHIFTED, ONE_SHIFTED};
	
	vector g = {playerx, playery + 6, ONE_SHIFTED, ONE_SHIFTED};
	bool grounded = Check(bounds, g).collided;
	bool ladder = Check(bounds, bounds).ladder;

	xv += key_tri_horz() * (ONE_SHIFTED / 32);
	//Only move up and down if we are on a ladder
	yv += -key_tri_vert() * (ONE_SHIFTED / 32) * ladder;

	if(key_tri_fire() > 0 && grounded && !ladder)
		yv += ((25<<SHIFT_AMOUNT) / 32);
	if (!grounded && !ladder)
		yv -= 88166 / 32; //(8166 = 1.3453 << 16)
	
	//Apply friction
	yv *= (ladder ? 851 : 951); yv /= 1000;
	xv *= (ladder ? 851 : 951); xv /= 1000;

	//Cap to 0.5
	xv = max(min(xv, (ONE_SHIFTED/2)), -(ONE_SHIFTED/2));

	g.x = bounds.x + xv; g.y = bounds.y - yv;
	vector v = Check(bounds, g).v;
	playerx = v.x; playery = v.y;

	//Stuff for next frame
	g.y = bounds.y;
	xv *= !Check(bounds, g).collided;
	g.y = playery - yv; g.x = bounds.x;
	yv *= !Check(bounds, g).collided;

}

void renderPlayer()
{
	//Lerp to player pos
	camerax += (playerx - camerax) / 4;
	cameray += (playery - cameray) / 4;

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
//Temp for now, just to test stuff
void resetPlayerPos()
{
	playerx = 0; playery = 0;
}

int main()
{
	//Initialise stuff
	init_map();

    irq_init(NULL);
    irq_enable(II_VBLANK);
	
	sioInit();

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
		loadChunks();
		processData();
		if(startMovement)
		{
			movement();
			sioMove();
		}
		
		renderPlayer();
	}
	return 0;
}
