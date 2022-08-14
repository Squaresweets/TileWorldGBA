#include "main.h"
#include "Colly.h"
#include "sio.h"
#include "TileMap.h"
#include "util.h"
#include "map.h"
#include "players.h"

#include <string.h>
#include <math.h>
#include <tonc.h>


#define CBB_0  0
#define SBB_0 28

//Screen offset from player
#define SCREEN_O_W    116
#define SCREEN_O_H    76

#define CROSS_TX 15
#define CROSS_TY 10

BG_POINT bg0_pt = { 0, 0 }; //For gameplay
BG_POINT bg1_pt = { 0, 0 }; //For minimap
SCR_ENTRY *bg0_map= se_mem[SBB_0];

OBJ_ATTR obj_buffer[128];
OBJ_ATTR *player = &obj_buffer[0];
OBJ_ATTR *hammer =  &obj_buffer[1];
OBJ_ATTR *indicator =  &obj_buffer[2];


//Fixed point, with a shift value of 16
int playerx = INITIAL_PLAYER_POS, playery = INITIAL_PLAYER_POS;
int camerax = INITIAL_PLAYER_POS, cameray = INITIAL_PLAYER_POS;
int mmcameray = 0;
int xv = 0, yv = 0;
bool startMovement = false;

//Values to do with placing tiles
u8 currentTileID = 0;
bool placeMode = false;
bool miniMapMode = false;
int tilex, tiley;

void init_map()
{
	// initialize a background
	REG_BG0CNT= BG_CBB(CBB_0) | BG_SBB(SBB_0) | BG_REG_64x64;
	REG_BG0HOFS= 0;
	REG_BG0VOFS= 0;
	bg0_pt.x = 0;
	bg0_pt.y = 0;

    memcpy(&tile_mem[CBB_0][0], TileMapTiles, TileMapTilesLen);
    memcpy(&tile_mem[4][0], spritesTiles, spritesTilesLen);
    memcpy(pal_bg_mem, TileMapPal, TileMapPalLen);
    memcpy(pal_obj_mem, spritesPal, spritesPalLen);

	//Code to load loading screen
	int ii, jj, c;
	SCR_ENTRY *pse= bg0_map;
	for(ii=0; ii<4; ii++)
	{
		for(jj=0; jj<32*32; jj++)
		{
			//Here we are hardcoding the loading screen
			//I KNOW this code is ugly, but in terms of saving storage
			//This makes the most sense
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
	//Quick optimisation, instead of dividing by 32 we shift right by 5
	vector bounds = {playerx, playery, ONE_SHIFTED, ONE_SHIFTED};
	
	vector g = {playerx, playery + 6, ONE_SHIFTED, ONE_SHIFTED};
	bool grounded = Check(bounds, g).collided;
	bool ladder = Check(bounds, bounds).ladder;

	//Only move if we are not in placeMode
	xv += key_tri_horz() * (ONE_SHIFTED >> 5) * !placeMode * !miniMapMode;
	//Only move up and down if we are on a ladder
	yv += -key_tri_vert() * (ONE_SHIFTED >> 5) * ladder * !placeMode * !miniMapMode;

	if(key_tri_fire() > 0 && grounded && !ladder && !placeMode && !miniMapMode)
		yv += ((25<<SHIFT_AMOUNT) >> 5);
	if (!grounded && !ladder)
	#if SHIFT_AMOUNT == 8
		yv -= 344 >> 5; //(344 = 1.3453 << SHIFT_AMOUNT)
	#elif SHIFT_AMOUNT == 16
		yv -= 88166 >> 5; //(88166 = 1.3453 << SHIFT_AMOUNT)
	#endif
	
	//Apply friction
	yv *= (ladder ? 871 : 973); yv >>= 10; //Divide by 1024
	xv *= (ladder ? 871 : 973); xv >>= 10;
	#if SHIFT_AMOUNT == 8
	xv *= (xv != -20); //If it is too small, just make it 0, this is to prevent sliding
	#endif
	//yv *= (yv != -20); //If it is too small, just make it 0, this is to prevent sliding

	//Cap to 0.5
	xv = max(min(xv, (ONE_SHIFTED >> 1)), -(ONE_SHIFTED >> 1));
	if(key_held(KEY_SELECT)) xv *= 5;

	g.x = bounds.x + xv; g.y = bounds.y - yv;
	vector v = Check(bounds, g).v;
	playerx = v.x; playery = v.y;
	if(playery > 2543 << SHIFT_AMOUNT)  playery = 2543 << SHIFT_AMOUNT; //Prevent clipping through bottom of world

	//Stuff for next frame
	g.y = bounds.y;
	xv *= !Check(bounds, g).collided;
	g.y = playery - yv; g.x = bounds.x;
	yv *= !Check(bounds, g).collided;
}

void render()
{
	//Lerp to player pos or tile pos
	camerax += ((placeMode ? (tilex & INT_MASK) : playerx) - camerax) / 4;
	cameray += ((placeMode ? (tiley & INT_MASK) : playery) - cameray) / 4;
	if(placeMode) //Lock camera and player to current screen during place mode
	{
		camerax = max(min(camerax, (((mapX-2)<<4))<<SHIFT_AMOUNT), ((mapX-4)<<4)<<SHIFT_AMOUNT); //32 is how far the screen can go right
		cameray = max(min(cameray, (((mapY-2)<<4))<<SHIFT_AMOUNT), ((mapY-4)<<4)<<SHIFT_AMOUNT); //32 is how far the screen can go down
		playery = min(playery, (((mapY-2)<<4)+10)<<SHIFT_AMOUNT); //Only thing which matters for the player since you can fall during placeMode
	}

	//Rendering player to screen
	//The -3 stuff is confusing, but basically just divides everything by the fixed point stuff to get the actual amount
	//And then times it by 8 (<<3) to get it how the gameboy likes it
	bg0_pt.x = (camerax>>(SHIFT_AMOUNT-3)) - SCREEN_O_W;
	bg0_pt.x &= 511;
	bg0_pt.y = (cameray>>(SHIFT_AMOUNT-3)) - SCREEN_O_H;

	//&511 is used to wrap the screen so as to prevent player sprites disapering when too far away from spawn
	u32 px = (playerx - (bg0_pt.x<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3) & 511;
	u32 py = (playery - (bg0_pt.y<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3) & 511;
	//To make sure the player doesn't rap around the screen, may be some easier way to do this with minmax
    if(px < 0 || px > SCREEN_W || py < 0 || py > SCREEN_H)     obj_hide(player);
	else                                                     obj_unhide(player, 0);

	if(miniMapMode) return;

	//Place player and indicator at correct position on the screen
    obj_set_pos(player, px, py);
    obj_set_pos(indicator, ((tilex & INT_MASK) - (bg0_pt.x<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3)& 511,
						   ((tiley & INT_MASK) - (bg0_pt.y<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3)& 511);
	RenderAllPlayers(bg0_pt);
	if(startMovement) oam_copy(oam_mem, obj_buffer, 19); 	// Update all sprites, I don't update map objects since they aren't changed during gameplay

	REG_BG_OFS[0]= bg0_pt;
}

void placeTiles()
{
	if(key_hit(KEY_B))
	{
		placeMode = !placeMode;
		//Following couple of lines turn on or off indicators (hammer and indicator)
		BFN_SET2(hammer->attr0, !placeMode * 0x0200, ATTR0_MODE);
		BFN_SET2(indicator->attr0, !placeMode * 0x0200, ATTR0_MODE);
		currentTileID = 0;

		tilex = playerx & INT_MASK;
		tiley = playery & INT_MASK; //Round player position to nearest tile
	}
	
	if(!placeMode) return;
	tilex += key_tri_horz() * (ONE_SHIFTED/6);
	tiley += key_tri_vert() * (ONE_SHIFTED/6);

	currentTileID = mod(currentTileID + bit_tribool(key_hit(-1), KI_R, KI_L), 12);
	indicator->attr2 = ATTR2_BUILD(currentTileID, 0, 0);
	if(key_hit(KEY_A)) //Time to place the tile!
		place((tilex-INITIAL_PLAYER_POS)>>SHIFT_AMOUNT,
			  (tiley-INITIAL_PLAYER_POS)>>SHIFT_AMOUNT, currentTileID); 
}
void handleMiniMap()
{
	if(key_hit(KEY_START))
	{
		miniMapMode = !miniMapMode;                   
		if(miniMapMode)
		{
			EnableMiniMapMode();
			REG_BG0CNT= BG_CBB(0) | BG_SBB(24) | BG_REG_64x64;
			bg1_pt.x = 0; 
			mmcameray = 40<<(SHIFT_AMOUNT-3);

			placeMode = false;
    		obj_hide_multi(obj_buffer, 19);
		}
		else
		{
			REG_BG0CNT= BG_CBB(0) | BG_SBB(SBB_0) | BG_REG_64x64;
			REG_BG_OFS[0]= bg0_pt;
		}
		oam_copy(oam_mem, obj_buffer, 19);
	}
	if(miniMapMode)
	{
		mmcameray += key_tri_vert() << SHIFT_AMOUNT;
		mmcameray = max(min(mmcameray, 80<<(SHIFT_AMOUNT-3)), 0);
		bg1_pt.y = mmcameray>>(SHIFT_AMOUNT-3);
		REG_BG_OFS[0]= bg1_pt;
	}
}

u16 pingtimer; //For how often I send pings

int main()
{
	//Initialise stuff
	init_map();

    irq_init(NULL);
    irq_enable(II_VBLANK);
	
	sioInit();

	REG_DISPCNT= DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D;
    oam_init(obj_buffer, 128);

	//There is probably a less messy way to do this
    obj_set_attr(player, 
        ATTR0_SQUARE,              // Square, regular sprite
        ATTR1_SIZE_8,              // 8x8p, 
        ATTR2_PALBANK(0) | 12);     // palbank 0, tile 12
    obj_set_attr(hammer, 
        ATTR0_SQUARE,              // Square, regular sprite
        ATTR1_SIZE_16,             // 16x16p, 
        ATTR2_PALBANK(0) | 14);    // palbank 0, tile 14
    obj_set_pos(hammer, 8, 136);
	obj_hide(hammer);
    obj_set_attr(indicator, 
        ATTR0_SQUARE,              // Square, regular sprite
        ATTR1_SIZE_8,              // 8x8p, 
        ATTR2_PALBANK(0) | 0);     // palbank 0, tile 1
	obj_hide(indicator);

	InitAllPlayers();

	while(1)
	{
		vid_vsync();
		key_poll();

		handle_serial();

		if(startMovement)
		{
			placeTiles();
			handleMiniMap();
			movement();
			if(pingtimer == 512) ping(); //Only ping every few seconds
			else pingtimer++;
			sioMove();

			loadChunks();
		}
		render();
	}
	return 0;
}
