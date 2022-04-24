# Attempt to figure out the server protocol
The server exchanges binary data with the client.

The first byte seems to be message type.

## 00
Unknown.

##(client) 01 (Connect)
The client sends this upon connecting. This isn't strictly required,
but the server won't send player join/leave events and 07 and such until it's
done.

    0x01
    0x01
	0x00

##(server) 01 (Player join)
The server sends this when the client is in range of another player.

	0x01
	Player ID (32 bit)
	X position (float32 big endian * 32)
	Y position (float32 big endian * 32)

##(server) 02 (Player leave)
The server sends this when another player moves out of range or leaves.

    0x02
    Player ID (32 bit)

##(Client) 03 (Ping)
The client sends this periodically to keep the connection alive.

	0x03

##(Server) 04 (Pong)
The server responds to pings with this.

	0x04
	
##(Client+Server) 05 (Place)
The server sends this when a client places a block. The client sends an
identical packet to place one.

    0x05
    X (int32 little endian)
    Y (int32 little endian)
    0x01 (??)
    Tile ID (8 bit)
    0x00 (??)


##(Client) 06 (Move)
The client sends this constantly when moving.

    0x06
    Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
    X (float32 big endian * 32)
    Y (float32 big endian * 32)
    X velocity (float32 big endian * 32)
    Y velocity (float32 big endian * 32)
##(Server) 06 (Move)
The server sends this when another player moves.

    0x06
    Keys (8 bit, up/down/left/right/jump are each mapped to a bit)
    X (float32 big endian * 32)
    Y (float32 big endian * 32)
    X velocity (float32 big endian * 32)
    Y velocity (float32 big endian * 32)
    Player ID (32 bit)
##(Server) 07 (Spawn tile data)
The server sends this when the client sends message ID 01 (Connect). Each byte
corresponds to a tile, and the width/height are 240. Centered on spawn,
presumably.

	0x07
	Tile ID (8 bit) * 57600

##(Client) 08 (Chunk request)
The client sends this to request a 3x15 or 15x3 chunk area on the minimap.
Chunks are 16x16.

Exact details on where this 3x15 or 15x3 area is pending, but it is essentially
the gap that would be made when the minimap scrolls (it does so every 3 chunks).

XDir and YDir are 0 and 1, 1 and 0, 0 and -1, or -1 and 0 based on the direction
of the area to be loaded. If it's in the +X direction, XDir is 1, and if it's in
the +Y direction YDir is 1, etc.

	0x08
	X (int32 little endian)
	Y (int32 little endian)
	XDir (int32 little endian)
	YDir (int32 little endian)

##(Server) 08 (Chunks)
The server sends this when the client requests chunks. Seems to always consist
of 45 chunks (a 3x15 or 15x3 area). Chunks are 16x16.

	0x08
	Chunk amount (uint32 little endian)
	Chunk data * Chunk amount
	(chunk format:
		ChunkX (int32 little endian)
		ChunkY (int32 little endian)
		Tile*256 (uint8)
	)

##(Client) 09 (Message)
The client sends this to send a chat message.

    0x09
    length (uint16 big endian)
    message (ascii)

##(Server) 09 (Incoming message)
The server sends this when a message is incoming. Some prior ones are also sent 
upon a client sending 01 (Connect).

	0x09
	name length (uint16 big endian)
	name (ascii)
	message length (8 bit)
	message (ascii)
