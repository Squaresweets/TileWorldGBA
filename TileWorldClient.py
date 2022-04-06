import usb.core
import usb.util
import signal
import sys
import time
import os
import websocket
import _thread
import rel
import multiboot

def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')

def send(data, epOut):
    print("SENDING: ", end="")
    for b in data.to_bytes(4, byteorder="big"):
        print("0x%02x " % b, end="")
    epOut.write(data.to_bytes(4, byteorder="big"))
    print("")


def read(epIn):
    recv = int.from_bytes(epIn.read(epIn.wMaxPacketSize, 100), byteorder='big')
    return recv


def readall(epIn):
    output = 0
    while True:
        try:
            output = output + read(epIn)
            output = output << 8
        except:
            break
    output = output >> 8
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
    multiboot.multiboot(epIn, epOut)


if __name__ == "__main__":
    main()