import os
import TileWorldClient
import time

#https://github.com/maciel310/gba-mmo-proxy/blob/main/multiboot.c


def multiboot(epIn, epOut):
    content = 0
    content = bytearray(open('TileWorldGBA_mb.gba', 'rb').read())
    fsize = os.path.getsize('TileWorldGBA_mb.gba')
    if fsize > 0x40000:
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
        out = (int(content[(i*2)])) + (int(content[(i*2)+1]) << 8)
        TileWorldClient.send(out, epOut)

    TileWorldClient.send(0x6200, epOut)
    TileWorldClient.send(0x6200, epOut)
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

    crcA = (token >> 16) & 0xFF
    seed = 0xFFFF00D1 | (crcA << 8)
    crcA = (crcA + 0xF) & 0xFF

    TileWorldClient.send((0x6400 | crcA), epOut)
    TileWorldClient.readall(epIn)

    fsize += 0xF
    fsize &= ~0xF

    TileWorldClient.send((fsize - 0x190) // 4, epOut)
    token = TileWorldClient.readall(epIn)
    crcB = (token >> 16) & 0xFF
    crcC = 0xC387
    print(fsize)
    print("Sending data!")
    for i in range(0xC0, fsize, 4):
        dat = int(content[i])
        dat += int(content[i + 1]) << 8
        dat += int(content[i + 2]) << 16
        dat += int(content[i + 3]) << 24

        tmp = dat

        for b in range(32):
            bit = (crcC ^ tmp) & 1
            if bit == 0:
                crcC = (crcC >> 1) ^ 0
            else:
                crcC = (crcC >> 1) ^ 0xc37b
            tmp >>= 1

        seed = seed * 0x6F646573 + 1
        dat = seed ^ dat ^ (0xFE000000 - i) ^ 0x43202F2F

        TileWorldClient.send(dat & 0xFFFFFFFF, epOut, False)
        chk = TileWorldClient.readall(epIn, False) >> 16

        if chk != (i & 0xFFFF):
            print("Transmission error at byte: " + str(i))
            exit()
        time.sleep(0.1)