#!/usr/bin/python
# Output: Bit pattern for bite stream
# | led | brightness | color1 | color2 |
# led is any number between 0 inclusive and C_NUMBER_OF_LIGHTS exclusive
# brightness is any number between 0 and 255
# color1 is the the packed red and green values the 4 high order bits are green and the 4 low order bits are red
# color2 is just the 4 low order blue bits
# Later on i will go back and wrap these in objects
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

###### Imports ######
import serial
import time      # Needed for some tests
import threading # Needed for testing of automated screen clear

###### Constants(sort of dont change while running code) #####
# Throw down some constants
C_BAUDRATE = 96000  #90000 #57600 #115200
C_NUMBER_OF_LIGHTS = 70
# Lookup table for mapping x,y cordnets to led numbers on the board
C_LOOKUP_TABLE = [[63,64,65,66,67,68,69],[62,61,60,59,58,57,56],[49,50,51,52,53,54,55],[48,47,46,45,44,43,42],[35,36,37,38,39,40,41],[34,33,32,31,30,29,28],[21,22,23,24,25,26,27],[20,19,18,17,16,15,14],[7,8,9,10,11,12,13],[6,5,4,3,2,1,0]]                         
C_LIGHT_BOARD_HEIGHT = 7
C_LIGHT_BOARD_WIDTH  = 10

###### Flags ########
F_DEBUG = False

########### Serial Port Stuff ############
#
##########################################
# Setup and open the Serial connection
ser = serial.Serial()
ser.baudrate = C_BAUDRATE
ser.port = "//dev/ttyACM2"
ser.xonxoff = False
ser.dsrdtr  = False
ser.dsrdtr  = False
ser.open()

def OpenSerial( deviceName ):
  ser.port = deviceName
  ser.open()

############# Low Level Functions ###################
# These have no error checking you have been warned
#####################################################
def write( led, color1, color2 ):
  ser.write( [ led, 0xFF, color1, color2 ] )
  if F_DEBUG == True:
    print( "led = " + str(led) + " brightness = 0xFF" + " color1 = " + str(color1) + " color2 = " + str(color2) )

def writeAll( color1, color2 ):
  for led in range(0, C_NUMBER_OF_LIGHTS):
    write( led, color1, color2 )
  
def writeRGBA( led, red, green, blue, brightness = 255 ):
  # Bitshift green value so we can add it to red for transmit
  green = green << 4
  ser.write( [ led, brightness, green + red, blue ] )
  if F_DEBUG == True:
    print( "led = " + str(led) + " red = " + str(red) + " blue = " + str(blue) + " green = " + str(green) )
    print( "Sending: led = " +  str(led) + " brightness = " + str(brightness) + " color1 = " + str(green + red) + " color2 = " + str(blue) )

def writeAllRGBA( red, green, blue, brightness = 255 ):
  for led in range(0, C_NUMBER_OF_LIGHTS):
    writeRGBA( led, red, green, blue, brightness )

########### Shortcut functions ###############
#
##############################################
def clearscreen():
  writeAllRGBA(0, 0, 0, 0)

def whitescreen():
  writeAllRGBA(15,15,15)

############ Turn the screen into a grid ##################
#
###########################################################
def writeXYRGBA( xCord, yCord, red, green, blue, brightness = 255 ):
  writeRGBA( C_LOOKUP_TABLE[xCord][yCord], red, green, blue, brightness )
  
def writeVertialLine( xCord, red, green, blue, brightness = 255 ):
  for y in range(0, C_LIGHT_BOARD_HEIGHT):
    writeXYRGBA( xCord, y, red, green, blue, brightness )

def writeHorozontalLine( yCord, red, green, blue, brightness = 255 ):
  for x in range(0, C_LIGHT_BOARD_WIDTH):
    writeXYRGBA( x, yCord, red, green, blue, brightness )

############# Frame Buffer ################################
# Frame Buffer needs xy support from above because I dont 
# want to try and debug it using the old pixel mapping..
# I also want it to be board agnostic
###########################################################
FrameBuffer       = [ [ [0, 0, 0, 0] for y in range(C_LIGHT_BOARD_HEIGHT) ] for x in range(C_LIGHT_BOARD_WIDTH) ]
C_BUFFER_RED        = 0
C_BUFFER_GREEN      = 1
C_BUFFER_BLUE       = 2
C_BUFFER_BRIGHTNESS = 3

# This could be done with a list better later as long as dynamic alocation doesn't become a slowdown
# A list will reduce the number of opps needed to render everything out though
PixelsMarkedDirty = [ [ False for j in range(C_LIGHT_BOARD_HEIGHT) ] for i in range(C_LIGHT_BOARD_WIDTH) ]
PixelsDirty = False
WholeBufferDirty = False

def display():
  global FrameBuffer
  global PixelsMarkedDirty
  global WholeBufferDirty
  global PixelsDirty

  if WholeBufferDirty == True:   # If we did something to make all the pixels dirty write them all out and clean up the dirty markers along the way
    for x in range(0, C_LIGHT_BOARD_WIDTH):
      for y in range(0, C_LIGHT_BOARD_HEIGHT):
        temp = FrameBuffer[x][y]
        writeXYRGBA( x, y, temp[C_BUFFER_RED], temp[C_BUFFER_GREEN], temp[C_BUFFER_BLUE], temp[C_BUFFER_BRIGHTNESS] )
        PixelsMarkedDirty[x][y] = False
    PixelsDirty = False
    WholeBufferDirty = False
    return

  # if we have dirty pixels write them out to the board and clean up the dirty markers
  if PixelsDirty == True:
    for x in range(0, C_LIGHT_BOARD_WIDTH):
      for y in range(0, C_LIGHT_BOARD_HEIGHT):
        if PixelsMarkedDirty[x][y] == True:
          # for each pixel in matrix that is dirty write out the pixel
          # clear the dirty pixel array
          temp = FrameBuffer[x][y]
          writeXYRGBA( x, y, temp[C_BUFFER_RED], temp[C_BUFFER_GREEN], temp[C_BUFFER_BLUE], temp[C_BUFFER_BRIGHTNESS] )
          PixelsMarkedDirty[x][y] = False  
    PixelsDirty = False 

def bufferedWriteXYRGBA( xCord, yCord, red, green, blue, brightness = 255 ):
  global FrameBuffer
  global PixelsMarkedDirty
  global PixelsDirty

  # Check to see if the pixel is already set as desired and if so jump out
  if FrameBuffer[xCord][yCord][C_BUFFER_RED] == red:
    if FrameBuffer[xCord][yCord][C_BUFFER_GREEN] == green:
      if FrameBuffer[xCord][yCord][C_BUFFER_BLUE]   == blue:
        if FrameBuffer[xCord][yCord][C_BUFFER_BRIGHTNESS] == brightness:
          return

  # Write to frame buffer 
  FrameBuffer[xCord][yCord][C_BUFFER_RED]         = red
  FrameBuffer[xCord][yCord][C_BUFFER_GREEN]       = green
  FrameBuffer[xCord][yCord][C_BUFFER_BLUE]        = blue
  FrameBuffer[xCord][yCord][C_BUFFER_BRIGHTNESS]  = brightness
  
  # Mark Pixel as dirty
  PixelsMarkedDirty[xCord][yCord] = True
  PixelsDirty = True

def bufferedWriteAllRGBA( red, green, blue, brightness = 255 ):
  global WholeBufferDirty
  
  # Set the flag so we know its all dirty
  WholeBufferDirty = True
  
  # Write out all the pixels to the buffer
  for x in range(0, C_LIGHT_BOARD_WIDTH):
    for y in range(0, C_LIGHT_BOARD_HEIGHT):
      bufferedWriteXYRGBA(x, y, red, green, blue, brightness)

def bufferedClearScreen():
  bufferedWriteAllRGBA( 0, 0, 0, 0 )

def bufferedWhiteScreen():
  bufferedWriteAllRGBA(15, 15, 15, 15 )


############ Test Functions ###############################
#
###########################################################
def testAllColors(led):
  for r in range( 0, 15 ):
    for b in range( 0, 15 ):
      for g in range( 0, 15 ):
        writeRGBA(led, r, g, b, 255)

def testWholeBoardAllTheColors():
  for r in range( 0, 15 ):
    for b in range( 0, 15 ):
      for g in range( 0, 15):
        writeAllRGBA(r, g, b, 128)
	
def testEachLightOnceUsingXY():
  # keep track of last write so we can clear it
  lastX = 0
  lastY = 0
  for y in range(0, C_LIGHT_BOARD_HEIGHT ):
    for x in range(0, C_LIGHT_BOARD_WIDTH ):
      writeXYRGBA( lastX, lastY, 0, 0, 0, 0)
      writeXYRGBA( x, y, 15, 15, 15)
      lastX = x
      lastY = y
      time.sleep(1)


def testXYSystem():
  # Set each pixel to red
  for x in range(0, C_LIGHT_BOARD_WIDTH ):
    for y in range(0, C_LIGHT_BOARD_HEIGHT ):
      writeXYRGBA( x, y, 15, 0, 0, 255 )

  # Clear for next text
  clearscreen()  

  testEachLightOnceUsingXY()


def TestBuffer():
  for x in range(0, C_LIGHT_BOARD_WIDTH):
    for y in range(0, C_LIGHT_BOARD_HEIGHT):
      bufferedWriteXYRGBA(x, y, 8, 8, 8)
  bufferedClearScreen()
  bufferedWhiteScreen()
  for x in range(0, C_LIGHT_BOARD_WIDTH):
    bufferedWriteXYRGBA(x, 4, 8, 8, 0)

def testDoTheDisplay():
  threading.Timer(0.04, testDoTheDisplay).start()
  display()

def testNew():
  for x in range(0, C_LIGHT_BOARD_WIDTH):
    for y in range(0, C_LIGHT_BOARD_HEIGHT):
      for g in range( 0, 13 ):
        bufferedWriteXYRGBA(x, y,0, g, 0)

