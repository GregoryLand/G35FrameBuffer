#!/usr/bin/python
#*************************************************************************************
# * Board Pattern 
# * 
# * |----|----|----|----|----|----|----|----|----|----|
# * | 69 | 56 | 55 | 42 | 41 | 28 | 27 | 14 | 13 |  0 |
# * | 68 | 57 | 54 | 43 | 40 | 29 | 26 | 15 | 12 |  1 |
# * | 67 | 58 | 53 | 44 | 39 | 30 | 25 | 16 | 11 |  2 |
# * | 66 | 59 | 52 | 45 | 38 | 31 | 24 | 17 | 10 |  3 |
# * | 65 | 60 | 51 | 46 | 37 | 32 | 23 | 18 |  9 |  4 |
# * | 64 | 61 | 50 | 47 | 36 | 33 | 22 | 19 |  8 |  5 |
# * | 63 | 62 | 49 | 48 | 35 | 34 | 21 | 20 |  7 |  6 |
# * |----|----|----|----|----|----|----|----|----|----|
#**************************************************************************************
####### Imports ######
import socket
import queue
import threading
import test


#**************************************************************************************
# Network Connection
#**************************************************************************************
def networkMain( incomingOrders ):
    while True:
        print('Network Thread')
    return 

#**************************************************************************************
# Serial Output to Arduino
#**************************************************************************************
def serialMain( incomingOrders ):
    while True:
        print('Serial Thread')
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
    
    # Kick off the threads
    networkThread.start()
    serialThread.start()
    
    #serialMain( incomingOrders ) # seems cleaner just to run serialMain on main thread
    networkThread.join()
    serialThread.join()
    

    

#**************************************************************************************
# Run script if we ran as script
#**************************************************************************************
if __name__ == "__main__":
    # execute only if run as a script
    main()
