import os
import TileWorldClient
import time
import math

#https://github.com/maciel310/gba-mmo-proxy/blob/main/multiboot.c


def multiboot(epIn, epOut, path):
    content = 0
    content = bytearray(open(path, 'rb').read())
    fsize = os.path.getsize(path)
    # Padding to avoid errors
    content = content.ljust(fsize + 64, b'\0')
    if fsize > 0x40000:
        print("File size error, max 256KB")
        exit()

    print("Turn on the GBA!")
    TileWorldClient.send(0x6200, epOut, False)
    TileWorldClient.readall(epIn, False)
    recv = 0
    while (recv >> 16) != 0x7202:
        for i in range(15):
            recv = TileWorldClient.sendread4(0x6200, epOut, epIn)
            if (recv >> 16) == 0x7202:
                break
        time.sleep(0.0625)

    print("Lets do this thing!")

    TileWorldClient.sendread4(0x6102, epOut, epIn)

    for i in range(96):
        out = (int(content[(i*2)])) + (int(content[(i*2)+1]) << 8)
        TileWorldClient.sendread4(out, epOut, epIn)

    TileWorldClient.sendread4(0x6200, epOut, epIn)
    TileWorldClient.sendread4(0x6202, epOut, epIn)
    TileWorldClient.sendread4(0x63D1, epOut, epIn)

    token = TileWorldClient.sendread4(0x63D1, epOut, epIn)
    if (token >> 24) != 0x73:
        print("Failed handshake!")
        multiboot(epIn, epOut, path)
        return
    else:
        print("Handshake successful!")

    time.sleep(0.0625)

    crcA = (token >> 16) & 0xFF
    seed = 0xFFFF00D1 | (crcA << 8)
    crcA = (crcA + 0xF) & 0xFF

    TileWorldClient.sendread4((0x6400 | crcA), epOut, epIn)

    fsize += 0xF
    fsize &= ~0xF

    token = TileWorldClient.sendread4((fsize - 0x190) // 4, epOut, epIn)
    crcB = (token >> 16) & 0xFF
    crcC = 0xC387
    print("Sending data, Length: " + str(fsize))
    start = time.time()
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

        TileWorldClient.sendread4(dat & 0xFFFFFFFF, epOut, epIn)

    tmp = 0xFFFF0000 | (crcB << 8) | crcA

    for b in range(32):
        bit = (crcC ^ tmp) & 1
        if bit == 0:
            crcC = (crcC >> 1) ^ 0
        else:
            crcC = (crcC >> 1) ^ 0xc37b
        tmp >>= 1

    TileWorldClient.sendread4(0x0065, epOut, epIn)
    while True:
        recv = TileWorldClient.sendread4(0x0065, epOut, epIn)
        if (recv >> 16) == 0x0075:
            break

    TileWorldClient.sendread4(0x0066, epOut, epIn)
    TileWorldClient.sendread4(crcC & 0xFFFF, epOut, epIn)
    print(f"{format(time.time() - start, '.3f')}s")
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~DONE!~~~~~~~~~~~~~~~~~~~~~~~~~~\n")
