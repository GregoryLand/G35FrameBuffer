#!/usr/bin/python
#*************************************************************************************
# The server expects a data packets based on the first byte sent
# 1) 1 unsigned byte for the function
#    value 0 = send colour
#    At a later time I will setup functions to return data from the server
#    that will tell you info about the board and its configuration such as number of lights on x or y 
# 2) 1 unsigned byte x cord valid numbers 1-255
# 3) 1 unsigned byte y cord valid numbers 1-255
# 4) 1 unsigned byte red valid numbers 1-15
# 5) 1 unsigned byte green valid numbers 1-15
# 6) 1 unsigned byte blue valid numbers 1-15
# 7) 1 unsigned byte brightness valid numbers 1-255
# Packed BIGENDIAN as all network data should be packed 
# http://en.wikipedia.org/wiki/Endianness
#**************************************************************************************

####### Imports ######
import socket
import struct
import queue
import threading
import test

F_DEBUG = False

#**************************************************************************************
# Network Connection
#**************************************************************************************
def networkMain( incomingOrders ):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as networkConnection:
        networkConnection.bind((socket.gethostname(), 6234))
        networkConnection.listen(1)

        # Enter waiting for connections loop
        while True:
            # Wait for connection
            print('Waiting For Connection')
            clientConnection, clientAddress = networkConnection.accept()
            
            # Connection made start processing 
            print('Connection Established From ' + str(clientAddress))     
            while True:
                if F_DEBUG:
                    print('Network Thread')
                    
                data = clientConnection.recv(7)

                if not data:
                    break
                else:
                    if F_DEBUG:
                        print( str(data) )
                    (order, x, y, red, green, blue, brightness) = struct.unpack_from('!BBBBBBB', data )
                    if order is 0: # send a color packet
                        incomingOrders.put( (x, y, red, green, blue, brightness) )
    return 

#**************************************************************************************
# Serial Output to Arduino
#**************************************************************************************
def serialMain( incomingOrders ):
    test.OpenSerial('//dev//ttyACM0')
    test.clearscreen()

    while True:
        (x, y, red, green, blue, brightness) = incomingOrders.get()
        
        if F_DEBUG:
            print( 'X: ' + str(x) +' Y: ' + str(y) + ' red: ' + str(red) + ' green: ' + str(green) + ' blue: ' + str(blue) + ' brightness: ' + str(brightness) )
            
        test.writeXYRGBA( x, y, red, green, blue, brightness )
    return

#**************************************************************************************
# Main
#**************************************************************************************
def main():
    # This FIFO is designed to multi threading and handles locks for us
	# We create the messages for the queue in networking main
	# and we processes and output serial information for that data in 
	# serialMain
    incomingOrders = queue.Queue()
    
	# Start both threads one to handle network packets and another
	# to handle serial output
    networkThread = threading.Thread(target=networkMain, args=(incomingOrders,))
    serialThread  = threading.Thread(target=serialMain, args=(incomingOrders,))
    
    # Set each thread to terminate when main does
    networkThread.daemon = True
    serialThread.daemon  = True
    
    # Kick off the threads
    networkThread.start()
    serialThread.start()
    
    #serialMain( incomingOrders ) # seems cleaner just to run serialMain on main thread
    networkThread.join()
    serialThread.join()
    return

    

#**************************************************************************************
# Run script if we ran as script
#**************************************************************************************
if __name__ == "__main__":
    # execute only if run as a script
    main()
