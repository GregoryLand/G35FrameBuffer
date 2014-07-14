import socket
import struct

def sendLightPacket( networkConnection, lightId, red, green, blue, brightness ):
    packetBytes = struct.pack( '!BBBBB', lightId, red, green, blue, brightness )
    print(str(packetBytes))
    networkConnection.send(packetBytes)
    return

#**************************************************************************************
# Main
#**************************************************************************************
def main():
    HOST = socket.gethostname()
    PORT = 6234

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as networkConnection:
        networkConnection.connect((HOST, PORT))
        while True:
            s = input('Press 1 - 0 to send test packet q key to close connection: ')
            if s is 'q':
                networkConnection.close()
                return
            elif s is '1':
                sendLightPacket( networkConnection, 0, 15, 15, 15, 255 )
            elif s is '2':
                sendLightPacket( networkConnection, 1, 15, 0, 0, 255 )
            elif s is '3':
                sendLightPacket( networkConnection, 2, 0, 15, 0, 255 )
            elif s is '4':
                sendLightPacket( networkConnection, 3, 0, 0, 15, 255 )
            elif s is '5':
                sendLightPacket( networkConnection, 4, 15, 15, 0, 255 )
            elif s is '6':
                sendLightPacket( networkConnection, 5, 15, 0, 15, 255 )
            elif s is '7':
                sendLightPacket( networkConnection, 6, 0, 15, 15, 255 )
            elif s is '8':
                sendLightPacket( networkConnection, 7, 15, 15, 15, 255 )
            elif s is '9':
                sendLightPacket( networkConnection, 8, 7, 7, 7, 255 )
            elif s is '0':
                sendLightPacket( networkConnection, 9, 10, 10, 10, 255 )
    return


#**************************************************************************************
# Run script if we ran as script
#**************************************************************************************
if __name__ == "__main__":
    # execute only if run as a script
    main()
