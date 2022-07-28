#include "players.h"
#include "Colly.h"
#include "main.h"
#include "util.h"
#include "map.h"

#include <tonc.h>
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
    int XV;
    int YV;
    u32 ID;
} oplayer;

oplayer players[16];


void UpdatePlayer(u8* packet)
{
    //Before we do anything we need to convert from floating point to fixed point and /32
    //CHECK IF THERE IS A PROBLEM WITH ALIGNMENT HERE
    *(u32*)(packet + 2)  = Float_to_fixed(*(float*)(packet + 2)) >> 5; //X
    *(u32*)(packet + 6)  = Float_to_fixed(*(float*)(packet + 6)) >> 5; //Y
    *(u32*)(packet + 10) = Float_to_fixed(*(float*)(packet + 10)) >> 5; //XV
    *(u32*)(packet + 12) = Float_to_fixed(*(float*)(packet + 12)) >> 5; //YV

    //Check if the player position is even in the bounds of the map
    if(*(u32*)(packet + 2) < camerax || *(u32*)(packet + 2) > camerax + (SCREEN_W << ONE_SHIFTED << 3)) return;
    if(*(u32*)(packet + 6) < cameray || *(u32*)(packet + 6) > cameray + (SCREEN_H << ONE_SHIFTED << 3)) return;

    u32 id = *(packet + 17); //The start of the u32 is 17 bytes into the packet
    u8 freeSlot = 105; //If it is more than 15 we know there are no free slots
    for(u8 i = 0; i < 16; i++)
    {
        if(!players[i].active){ freeSlot = i; continue; }
        
        if(players[i].ID == id) //The player we got movement info from is already in the array, woo!
        {
            memcpy(&(players[i]) + 3, packet + 1, 18); //Update the data already there ʕ•ᴥ•ʔ
            PlayerFloatToFixed(i);
            return;
        }
    }
    //Right, the player ID is not already in the array
    if(freeSlot > 15) return; //No free slots, oh well ¯\_(ツ)_/¯

    memcpy(&(players[freeSlot]) + 3, packet + 1, 18); //Put the data into the right place (ﾉ◕ヮ◕)ﾉ*:･ﾟ✧
    PlayerFloatToFixed(freeSlot);
    players[freeSlot].active = true;
    //We only actually move the players later
}
void PlayerLeave(u32 id)
{
    for(u8 i = 0; i < 16; i++)
        if(players[i].ID == id) { players[i].active = false; return; }
    //Should never get here, fingers crossed we don't
}


void MovePlayers()
{
    for(u8 i = 0; i < 16; i++)
    {
        if(!players[i].active) continue;
        oplayer* p = &players[i];
        //Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
        s8 keysX = (p->keys & (1 << 3)) - (p->keys & (1 << 2));
        s8 keysY = (p->keys & (1 << 0)) - (p->keys & (1 << 1));


        vector bounds = {p->X, p->Y, ONE_SHIFTED, ONE_SHIFTED};
        
        vector g = {p->X, p->Y + 6, ONE_SHIFTED, ONE_SHIFTED};
        bool grounded = Check(bounds, g).collided;
        bool ladder = Check(bounds, bounds).ladder;

        //Only move if we are not in placeMode
        p->XV += keysX * (ONE_SHIFTED >> 5) * !placeMode;
        //Only move up and down if we are on a ladder
        p->YV += -keysY * (ONE_SHIFTED >> 5) * ladder * !placeMode;

        if((p->keys & (1 << 4)) && grounded && !ladder && !placeMode)
            p->YV += ((25<<SHIFT_AMOUNT) >> 5);
        if (!grounded && !ladder)
            p->YV -= 88166 >> 5; //(88166 = 1.3453 << SHIFT_AMOUNT)
        
        //Apply friction
        p->YV *= (ladder ? 871 : 973); yv >>= 10; //Divide by 1024
        p->XV *= (ladder ? 871 : 973); xv >>= 10;

        //Cap to 0.5
        p->XV = max(min(p->XV, (ONE_SHIFTED >> 1)), -(ONE_SHIFTED >> 1));

        g.x = bounds.x + xv; g.y = bounds.y - yv;
        vector v = Check(bounds, g).v;
        p->X = v.x; p->Y = v.y;

        //Stuff for next frame
        g.y = bounds.y;
        p->XV *= !Check(bounds, g).collided;
        g.y = p->Y - p->YV; g.x = bounds.x;
        p->YV *= !Check(bounds, g).collided;
    }
}