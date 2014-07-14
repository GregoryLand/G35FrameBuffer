#!/usr/bin/python
import socket
import struct
import argparse

##############################################################################
# The server expects a data packets based on the first byte sent
# 1) 1 unsigned byte for the function
#    values = 0 send color
# 2) 1 unsigned byte x cord valid numbers 1-255
# 3) 1 unsigned byte y cord valid numbers 1-255
# 4) 1 unsigned byte red valid numbers 1-15
# 5) 1 unsigned byte green valid numbers 1-15
# 6) 1 unsigned byte blue valid numbers 1-15
# 7) 1 unsigned byte brightness valid numbers 1-255
# Packed BIGENDIAN as all network data should be packed 
# http://en.wikipedia.org/wiki/Endianness
##############################################################################

def sendLightPacket( networkConnection, x, y, red, green, blue, brightness ):
    packetBytes = struct.pack( '!BBBBBBB', 0, x, y, red, green, blue, brightness )
    print(str(packetBytes))
    networkConnection.send(packetBytes)
    return

#**************************************************************************************
# Main
#**************************************************************************************
def main( hostname ):
    HOST = socket.gethostbyname(hostname)
    PORT = 6234

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as networkConnection:
        networkConnection.connect((HOST, PORT))
        while True:
            s = input('Press 1 - 0 to send test packet q key to close connection: ')
            if s is 'q':
                networkConnection.close()
                return
            elif s is '1':
                sendLightPacket( networkConnection, 0, 0, 15, 15, 15, 255 )
            elif s is '2':
                sendLightPacket( networkConnection, 0, 1, 15, 0, 0, 255 )
            elif s is '3':
                sendLightPacket( networkConnection, 0, 2, 0, 15, 0, 255 )
            elif s is '4':
                sendLightPacket( networkConnection, 0, 3, 0, 0, 15, 255 )
            elif s is '5':
                sendLightPacket( networkConnection, 0, 4, 15, 15, 0, 255 )
            elif s is '6':
                sendLightPacket( networkConnection, 0, 5, 15, 0, 15, 255 )
            elif s is '7':
                sendLightPacket( networkConnection, 0, 6, 0, 15, 15, 255 )
            elif s is '8':
                sendLightPacket( networkConnection, 0, 7, 15, 15, 15, 255 )
            elif s is '9':
                sendLightPacket( networkConnection, 0, 8, 7, 7, 7, 255 )
            elif s is '0':
                sendLightPacket( networkConnection, 0, 9, 10, 10, 10, 255 )
    return


#**************************************************************************************
# Run script if we ran as script
#**************************************************************************************
if __name__ == "__main__":
    # execute only if run as a script
    parser = argparse.ArgumentParser()#(description='This is a demo client for the lightboard')
    parser.add_argument("hostname", help="The hostname of the lightboard")
    args = parser.parse_args()

    main(args.hostname)
    