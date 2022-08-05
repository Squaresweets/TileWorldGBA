#include "players.h"
#include "Colly.h"
#include "main.h"
#include "util.h"
#include "map.h"
#include "sio.h"

#include <string.h>
#include <tonc.h>
#include <stdlib.h>
/*
BOYS, THE PLAN IS SIMPLE!


We have a struct to contain information about a player which is currently on screen
Each "active" struct corresponds to a player sprite, tho which corresponds to which
Doesn't matter since the player sprite is always the same

The structs are in an array of 16, if there are more than 16 players ¯\_(ツ)_/¯

Struct has:
    1 byte acting as a boolean to show whether this struct is "active"
    2 bytes PADDING *********** (to make everything align)
    1 byte for keys
    4 bytes X
    4 bytes Y
    4 bytes XV
    4 bytes YV
    4 bytes ID

Since the server sends us data like this:
    0x06
    Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
    X (float32 big endian * 32)
    Y (float32 big endian * 32)
    X velocity (float32 big endian * 32)
    Y velocity (float32 big endian * 32)
    Player ID (32 bit)
We can memcpy the last 20 bytes into the right position when we get data through

After moving the player, we loop through each of the 16 structs and check if it is active and on screen
If it is we do physics calculations on it using colly *MAYBE*

We then render each of the players


We do not worry about player join commands, just player moves and player leaves
*/

typedef struct OPlayer //(Other player)
{
    bool active;
    u8 padding[2];
    u8 keys;
    int X;
    int Y;
    u32 ID;
} oplayer;

oplayer players[16];

//********************************** POOLING AND MOVEMENT LOGIC **********************************
void UpdatePlayer(u8* packet)
{
    //No need to worry about velocity since we are not doing stuff with that
    u32 id; memcpy(&id, (packet+18), 4); //The start of the u32 is 18 bytes into the packet

    //Before we do anything we need to convert from floating point to fixed point and /32
    //Sorry for the annoying code here, if you have any better ideas please let me know
    float Xf; memcpy(&Xf, (packet+2), 4); int X  = Float_to_fixed(Xf) >> 5;
    float Yf; memcpy(&Yf, (packet+6), 4); int Y  = Float_to_fixed(Yf) >> 5;

    //Check if the player position is even in the bounds of the camera
    if(X + INITIAL_PLAYER_POS < (((mapX-5)*16) << SHIFT_AMOUNT) || X + INITIAL_PLAYER_POS > (((mapX-1)*16) << SHIFT_AMOUNT)   ||
       Y + INITIAL_PLAYER_POS < (((mapY-5)*16) << SHIFT_AMOUNT) || Y + INITIAL_PLAYER_POS > (((mapY-1)*16) << SHIFT_AMOUNT)) { PlayerLeave(id); return; }

    u8 freeSlot = 100; //If it is more than 15 we know there are no free slots
    for(u8 i = 0; i < 16; i++)
    {
        if(!players[i].active){ freeSlot = i; continue; }
        
        if(players[i].ID == id) //The player we got movement info from is already in the array, woo!
        {
            //Update the data already there ʕ•ᴥ•ʔ
            players[i].X = X;   players[i].Y = Y;
            players[i].ID = id; players[i].keys = packet[1];
            return;
        }
    }
    //Right, the player ID is not already in the array
    if(freeSlot > 15) return; //No free slots, oh well ¯\_(ツ)_/¯

    players[freeSlot].X = X;   players[freeSlot].Y = Y;
    players[freeSlot].ID = id; players[freeSlot].keys = packet[1];
    players[freeSlot].active = true;
    //We only actually move the players later
}

void PlayerLeave(u32 id)
{
    for(u8 i = 0; i < 16; i++)
        if(players[i].ID == id) { players[i].active = false; return; }
}

//********************************** RENDERING **********************************
void InitAllPlayers()
{
    for(u8 i = 0; i < 16; i++)
    {
        obj_set_attr(&obj_buffer[i+3], //+3 due to player, hammer + indicator sprites
            ATTR0_SQUARE,              // Square, regular sprite
            ATTR1_SIZE_8,              // 8x8p, 
            ATTR2_PALBANK(0) | 12);     // palbank 0, tile 12
	    obj_hide(&obj_buffer[i+3]);    //Start all sprites off hidden
    }
}
void RenderAllPlayers(BG_POINT bg0_pt)
{
    int x, y;
    for(u8 i = 0; i < 16; i++)
    {
        if(!players[i].active)                             {obj_hide(&obj_buffer[i+3]); continue; }
        x = (players[i].X + INITIAL_PLAYER_POS - (ONE_SHIFTED/2) - (bg0_pt.x<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3);
        y = (players[i].Y + INITIAL_PLAYER_POS - (ONE_SHIFTED/2) - (bg0_pt.y<<(SHIFT_AMOUNT-3)))>>(SHIFT_AMOUNT-3);
        if(x < 0 || x > SCREEN_W || y < 0 || y > SCREEN_H) {obj_hide(&obj_buffer[i+3]); continue; } //So the player doesn't rap around
        obj_unhide(&obj_buffer[i+3], 0);
        obj_set_pos(&obj_buffer[i+3], x, y);
    }
}