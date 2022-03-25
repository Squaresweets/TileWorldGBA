import usb.core
import usb.util
import signal
import sys
import time
import os
import websocket
import _thread
import rel

def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')

def send(data, epOut):
    print("SENDING: ", end="")
    for b in data:
        print("0x%02x " % b, end="")
    epOut.write(data)
    print("")


def read(epIn):
    recv = int.from_bytes(epIn.read(epIn.wMaxPacketSize, 100), byteorder='big')
    print("0x%02x " % recv)
    return recv


def clearbuffer(epIn):
    while True:
        try:
            read(epIn)
        except:
            break

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



    while True:
        epOut.write(bytearray(b'\x88'))
        time.sleep(0)
        clearbuffer(epIn);


def on_message(ws, message):
    #print(bytearray(message).hex())
    print(", ".join(hex(b) for b in message))


def on_error(ws, error):
    print(error)


def on_close(ws, close_status_code, close_msg):
    print("### closed ###")


def on_open(ws):
    print("Opened connection")


if __name__ == "__main__":
    #rel.safe_read()
    #websocket.enableTrace(True)
    #ws = websocket.WebSocketApp("wss://tileworld.org:7364",
                              #on_open=on_open,
                              #on_message=on_message,
                              #on_error=on_error,
                              #on_close=on_close)
    #ws.run_forever(dispatcher=rel)
    #rel.signal(2, rel.abort)  # Keyboard Interrupt
    #rel.dispatch()
    main()