import usb.core
import usb.util
import signal
import sys
import time
import os
import websocket
import rel
import multiboot
import threading

def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')

def send(data, epOut, debug = True, length = 4):
    epOut.write(0x3, data.to_bytes(length, byteorder="big"))
    if not debug:
        return
    print("SENDING: ", end="")
    for b in data.to_bytes(length, byteorder="big"):
        print("0x%02x " % b, end="")
    print("")

def send64(data, epOut, debug = True):
    epOut.write(0x3, data.to_bytes(64, byteorder="big"))
    if not debug:
        return
    print("SENDING: ", end="")
    for b in data.to_bytes(64, byteorder="big"):
        print("0x%02x " % b, end="")
    print("")

def read(epIn):
    recv = int.from_bytes(epIn.read(64, 10), byteorder='big')
    return recv


def readall(epIn, debug = True):
    output = 0
    while True:
        try:
            output = output + read(epIn)
            output = output << 8
        except:
            break
    output = output >> 8
    if debug:
        print("0x%02x " % output)
    return output

def main():
    signal.signal(signal.SIGINT, signal_handler)

    dev = None
    devices = list(usb.core.find(find_all=True, idVendor=0xcafe, idProduct=0x4011))
    for d in devices:
        dev = d

    if dev is None:
        raise ValueError('Device not found')

    reattach = False
    if (os.name != "nt"):
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
    #multiboot.multiboot(epIn, dev, "TileWorldGBA_mb.gba")

    """
    Ok, now to design my own protocol to make sure all data has been sent safely
    Someone has probably made something like this already, but It seems like fun
    
    We always send 32 bits at a time, since I want to be sending in bytes we can use the first byte
    as a header
    
    00000000
    This header will confirm recieving the previous message, and then the other 3 bytes will be the actual
    payload
    
    Bit 1: The first bit should say that we are actually confirming a message, if no message is recieved there is no
    point in confirming a message
    Bit 2-4: Packet number
    
    Bit 4-8: checksum for the current message
    
    """
    websocket.enableTrace(False)
    ws = websocket.WebSocketApp("wss://tileworld.org:7364", on_message=on_message)
    ws.run_forever(dispatcher=rel)

    ws.send(b'\x01\x01\x00', websocket.ABNF.OPCODE_BINARY)
    ping(ws)

    rel.signal(2, rel.abort)
    rel.dispatch()

def ping(ws):
    ws.send(b'\x03', websocket.ABNF.OPCODE_BINARY)
    threading.Timer(1, ping, [ws]).start()

def on_message(ws, message):
    print(message.hex())

if __name__ == "__main__":
    main()