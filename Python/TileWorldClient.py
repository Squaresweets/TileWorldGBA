import usb.core
import usb.util
import signal
import sys
import time
import os
import websocket
import rel
import multiboot
import asyncio
from threading import Thread

outbuf = []


#<editor-fold desc="Send and recieve functions">
def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')


def send(data, epOut, debug = True, length = 4):
    epOut.write(data.to_bytes(length, byteorder="big"))

    if not debug:
        return
    print("SENDING: ", end="")
    for b in data.to_bytes(length, byteorder="big"):
        print("0x%02x " % b, end="")
    print("")


def sendread4(data, epOut, epIn, debug = True, length = 4): #Just used for multiboot
    send(data, epOut, False, length)
    return read4(epIn)


def read(epIn):
    recv = int.from_bytes(epIn.read(4, 100), byteorder='big')
    return recv

def read4(epIn):
    output = 0
    i = 0
    while i < 4:
        try:
            output += int.from_bytes(epIn.read(1, 100), byteorder='big') << (3-i)*8
            i += 1
        except:
            pass

    return output


def readall(epIn, debug = True):
    output = 0
    while True:
        try:
            output = output + int.from_bytes(epIn.read(64, 10), byteorder='big')
            output = output << 8
        except:
            break
    output = output >> 8
    if debug:
        print("0x%02x " % output)
    return output
#</editor-fold>


def main():
    #<editor-fold desc="Setting up USB">
    signal.signal(signal.SIGINT, signal_handler)

    dev = None
    devices = list(usb.core.find(find_all=True, idVendor=0xcafe, idProduct=0x4011))
    for d in devices:
        dev = d

    if dev is None:
        raise ValueError('Device not found')

    if os.name != "nt":
        if dev.is_kernel_driver_active(0):
            try:
                reattach = True
                dev.detach_kernel_driver(0)
                print("kernel driver detached")
            except usb.core.USBError as e:
                sys.exit("Could not detach kernel driver: %s" % str(e))
        else:
            print("no kernel driver attached")

    dev.reset()

    dev.set_configuration()

    cfg = dev.get_active_configuration()

    intf = cfg[(2,0)]   # Or find interface with class 0xff

    epIn = usb.util.find_descriptor(
        intf,
        custom_match = \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_IN)
    assert epIn is not None
    epOut = usb.util.find_descriptor(
        intf,
        # match the first OUT endpoint
        custom_match = \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)
    assert epOut is not None

    # Control transfer to enable webserial on device
    dev.ctrl_transfer(bmRequestType = 1, bRequest = 0x22, wIndex = 2, wValue = 0x01)
    #</editor-fold>

    multiboot.multiboot(epIn, epOut, os.path.dirname(os.getcwd()) + "/TileWorldGBA_mb.gba")
    time.sleep(5)
    readall(epIn, False)

    ws = websocket.WebSocket()
    ws.connect("wss://tileworld.org:7364")
    # Setup thread so we can listen at the same time as the main logic loop
    t = Thread(target=listen,args=(ws,))
    t.start()

    # For incoming data
    i = 0
    expectedlen = 0
    incomingbuf = []

    # For outgoing data
    j = 0
    outlen = 0

    # Tell GBA it can start sending data
    send(0xDEADBEEF, epOut, False)
    read4(epIn)

    # Main logic loop
    while True:
        # ============= Tileworld->GBA =============
        if outlen == 0 and len(outbuf) > 0:
            # new data to send
            print("Sending len: " + str(len(outbuf[0])) + " to GBA")
            send(len(outbuf[0]), epOut, False)
            outlen = len(outbuf[0])
            #Padding to avoid errors
            outbuf[0] = outbuf[0].ljust(outlen + 16, b'\0')
            j = 0
        elif outlen > 0:
            # sending data
            out = (outbuf[0])[j] << 24
            out += (outbuf[0])[j+1] << 16
            out += (outbuf[0])[j+2] << 8
            out += (outbuf[0])[j+3]
            j += 4
            send(out, epOut, True)
            if j >= outlen:
                outlen = 0
                j = 0
                del outbuf[0]
        else:
            # no data to send, just send a blank packet
            send(0, epOut, False)

        # ============= GBA->Tileworld =============
        data = read4(epIn)
        if expectedlen == 0:
            expectedlen = data  # in bytes
            i = 0
            continue

        incomingbuf.append((data >> 24) & 0xFF)
        incomingbuf.append((data >> 16) & 0xFF)
        incomingbuf.append((data >> 8) & 0xFF)
        incomingbuf.append(data & 0xFF)
        i += 4

        if i >= expectedlen:
            incomingbuf = incomingbuf[:expectedlen]
            ba = bytearray(incomingbuf)
            print("Sending to TileWorld server: " + ba.hex())
            if ba[0] == 10: # This is a debug message
                del ba[0]
                print("DEBUG MESSAGE: ", end="")
                try:
                    print(str(ba, 'utf-8'))
                except:
                    print("Invalid message")
            else:
                ws.send(ba, websocket.ABNF.OPCODE_BINARY)


            incomingbuf = []
            expectedlen = 0
            i = 0


def listen(ws):
    # print("Started thread")
    while True:
        m = ws.recv()
        try:
            message = bytearray(m)
        except:
            print(m)
            exit()
        print("From tileworld server: " + message.hex())
        outbuf.append(message)


if __name__ == "__main__":
    main()