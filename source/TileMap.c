#include <tonc.h>
//{{BLOCK(TileMap)

//======================================================================
//
//	TileMap, 32x32@4, 
//	+ palette 16 entries, not compressed
//	+ 16 tiles not compressed
//	Total size: 32 + 512 = 544
//
//	Time-stamp: 2022-02-04, 22:21:10
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================


const unsigned int TileMapTiles[128] __attribute__((aligned(4)))=
{
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,
	0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,
	0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,0x33333333,
	0x44444444,0x44444444,0x44444444,0x44444444,0x44444444,0x44444444,0x44444444,0x44444444,
	0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,0x55555555,
	0x66666666,0x66666666,0x66666666,0x66666666,0x66666666,0x66666666,0x66666666,0x66666666,
	0x77777777,0x77777777,0x77777777,0x77777777,0x77777777,0x77777777,0x77777777,0x77777777,

	0x88888888,0x88888888,0x88888888,0x88888888,0x88888888,0x88888888,0x88888888,0x88888888,
	0x99999999,0x99999999,0x99999999,0x99999999,0x99999999,0x99999999,0x99999999,0x99999999,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0x0BCCCCB0,0x0C0000C0,0x0BCCCCB0,0x0C0000C0,0x0BCCCCB0,0x0C0000C0,0x0BCCCCB0,0x0C0000C0,
	0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,0xDDDDDDDD,
	0xDDD88DDD,0xD75888DD,0xD5558DDD,0x88587DDD,0x888777DD,0xD8DD777D,0xDDDDD777,0xDDDDDD77,
	0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,
	0x11111111,0x15111151,0x15111151,0x15111151,0x11111111,0x11511511,0x11155111,0x11111111,
};

const unsigned int TileMapPal[8] __attribute__((aligned(4)))=
{
	0x0A957B2F,0x48A6281C,0x08422BF0,0x4A527DE6,0x2A8508EB,0x01B17C1F,0x7FFF0236,0x000076CB,
};

//Despite the extra size I also have sprites with a white boarder, just for convenience
const unsigned int spritesTiles[144] __attribute__((aligned(4)))=
{
	0xEEEEEEEE,0xE111111E,0xE111111E,0xE111111E,0xE111111E,0xE111111E,0xE111111E,0xEEEEEEEE,
	0xEEEEEEEE,0xE222222E,0xE222222E,0xE222222E,0xE222222E,0xE222222E,0xE222222E,0xEEEEEEEE,
	0xEEEEEEEE,0xE333333E,0xE333333E,0xE333333E,0xE333333E,0xE333333E,0xE333333E,0xEEEEEEEE,
	0xEEEEEEEE,0xE444444E,0xE444444E,0xE444444E,0xE444444E,0xE444444E,0xE444444E,0xEEEEEEEE,
	0xEEEEEEEE,0xE555555E,0xE555555E,0xE555555E,0xE555555E,0xE555555E,0xE555555E,0xEEEEEEEE,
	0xEEEEEEEE,0xE666666E,0xE666666E,0xE666666E,0xE666666E,0xE666666E,0xE666666E,0xEEEEEEEE,
	0xEEEEEEEE,0xE777777E,0xE777777E,0xE777777E,0xE777777E,0xE777777E,0xE777777E,0xEEEEEEEE,
	0xEEEEEEEE,0xE888888E,0xE888888E,0xE888888E,0xE888888E,0xE888888E,0xE888888E,0xEEEEEEEE,

	0xEEEEEEEE,0xE999999E,0xE999999E,0xE999999E,0xE999999E,0xE999999E,0xE999999E,0xEEEEEEEE,
	0xEEEEEEEE,0xEAAAAAAE,0xEAAAAAAE,0xEAAAAAAE,0xEAAAAAAE,0xEAAAAAAE,0xEAAAAAAE,0xEEEEEEEE,
	0xEEEEEEEE,0xEBBBBBBE,0xEBBBBBBE,0xEBBBBBBE,0xEBBBBBBE,0xEBBBBBBE,0xEBBBBBBE,0xEEEEEEEE,
	0xEEEEEEEE,0xEC1111CE,0xEDCCCCDE,0xEC1111CE,0xEDCCCCDE,0xEC1111CE,0xEDCCCCDE,0xEEEEEEEE,
	0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,0xEEEEEEEE,
	0xEEEEEEEE,0xE000000E,0xE000000E,0xE000000E,0xE000000E,0xE000000E,0xE000000E,0xEEEEEEEE,
	0x00000000,0xE0000000,0xFE000000,0xFFEE0000,0xFF68E000,0xF666E000,0x666FFE00,0xF6FFFFE0,
	0x0000000E,0x000000EF,0x00000EFF,0x0000EFFF,0x0000EFFF,0x00000EFF,0x000000E6,0x00000E88,

	0x86FFFFFE,0x8EFFFFE0,0xE0EFFE00,0x000EE000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x0000E888,0x000E8888,0x00E88888,0x0E88888E,0xE88888E0,0xE8888E00,0xE888E000,0xEEEE0000,
};

const unsigned int spritesPal[8] __attribute__((aligned(4)))=
{
	0x7B2F7C1F,0x281C0A95,0x2BF048A6,0x7DE60842,0x08EB4A52,0x7C1F2A85,0x01B10236,0x08EC7FFF,
};

//Tilemap for LOADING on the menu
//It is split into two parts because of there being multiple screen blocks
//There may be a better way to do this, (there probably is tbh)
const u8 LOAD[4][13] =
{
	{1,0,0,0,1,1,1,0,1,1,1,0,1},
	{1,0,0,0,1,0,1,0,1,0,1,0,1},
	{1,0,0,0,1,0,1,0,1,1,1,0,1},
	{1,1,1,0,1,1,1,0,1,0,1,0,1}
};
const u8 DING[4][14] =
{
	{1,0,0,1,1,1,0,1,1,1,0,1,1,1},
	{0,1,0,0,1,0,0,1,0,1,0,1,0,0},
	{0,1,0,0,1,0,0,1,0,1,0,1,0,1},
	{1,1,0,1,1,1,0,1,0,1,0,1,1,1}
};