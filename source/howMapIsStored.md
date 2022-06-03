
    Tileworld data is orgnaised in 225 16x16 blocks
    0   1   2   3...
    15  16  17  18...
    etc.

    Tilemap displayed is organised in 4 32x32 blocks               
    0 1
    2 3

    To get these to line up I split each 32x32 block in the tilemap into 4 16x16
    0  1    4  5
    2  3    6  7

    8  9    12 13
    10 11   14 15
    (Keep in mind each of these is 16x16 (One chunk))


    So for the following function I should give it a memory address in the map array for the chunk I wanna copy
    And then give me an ID 0-15 for one of the chunks as seen above to copy it into
    

    First, I wanna get given the ID 0-15 in this pattern
    0  1    2  3
    4  5    6  7

    8  9    10 11
    12 13   14 15
    And then convert it to the other pattern
    Now I could do logic for this, but a conversion table is easier

(This was originally a comment in sio.c, but I moved it here to make the file a bit more usable)