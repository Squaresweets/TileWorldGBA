import os
import TileWorldClient

#https://github.com/maciel310/gba-mmo-proxy/blob/main/multiboot.c

def multiboot(epIn, epOut):
    content = 0
    with open('TileWorldGBA_mb.gba', 'rb') as f:
        content = bytearray(f.read())
    if os.path.getsize('TileWorldGBA_mb.gba') > 0x40000:
        print("File size error, max 256KB")
        exit()

    recv = 0
    while True:
        TileWorldClient.send(0x6202, epOut)
        recv = TileWorldClient.readall(epIn)
        if (recv >> 16) == 0x7202:
            break
    print("Lets do this thing!")
    TileWorldClient.send(0x6102, epOut)

    for i in range(96):
        out = (int(content[(i*2)]) << 8) + int(content[(i*2)+1])
        TileWorldClient.send(out, epOut)

    TileWorldClient.send(0x6200, epOut)
    TileWorldClient.send(0x6202, epOut)
    TileWorldClient.send(0x63D1, epOut)
    #Clear buffer
    TileWorldClient.readall(epIn)

    TileWorldClient.send(0x63D1, epOut)
    token = TileWorldClient.readall(epIn)
    if (token >> 24) != 0x73:
        print("Failed handshake!")
        exit()
    else:
        print("Handshake successful!")