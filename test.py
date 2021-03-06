#!/usr/bin/python
# Output: Bit pattern for bite stream
# | led | brightness | color1 | color2 |
# led is any number between 0 inclusive and C_NUMBER_OF_LIGHTS exclusive
# brightness is any number between 0 and 255
# color1 is the the packed red and green values the 4 high order bits are green and the 4 low order bits are red
# color2 is just the 4 low order blue bits
# Later on I will go back and wrap these in objects
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
#C_BAUDRATE = 28800 #57600 #115200 #96000  #90000 #57600 #115200
#C_BAUDRATE = 9600
#C_BAUDRATE = 19200
C_BAUDRATE = 28800
#C_BAUDRATE = 31000
#C_BAUDRATE = 32000
#####C_BAUDRATE = 32750  # 11.67 Frames a second pushing full frame
##NOT WORKING###############
#C_BAUDRATE = 38400
#C_BAUDRATE = 57600
#C_BAUDRATE = 115200
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
ser.port = "//dev//ttyACM0"
ser.xonxoff = False
ser.dsrdtr  = False
ser.dsrdtr  = False
#ser.open()

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
  ser.write( [ led, brightness, green + red, blue] )
  #ser.write( [ blue ] )
  if F_DEBUG == True:
    print( "led = " + str(led) + " red = " + str(red) + " green = " + str(green) + " blue = " + str(blue) )
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
# Frame Buffer needs xy support from above because I don't 
# want to try and debug it using the old pixel mapping..
# I also want it to be board agnostic
###########################################################
FrameBuffer       = [ [ [0, 0, 0, 0] for y in range(C_LIGHT_BOARD_HEIGHT) ] for x in range(C_LIGHT_BOARD_WIDTH) ]
C_BUFFER_RED        = 0
C_BUFFER_GREEN      = 1
C_BUFFER_BLUE       = 2
C_BUFFER_BRIGHTNESS = 3
C_MAX_FRAMES_BETWEEN_FULL_SYNC = 48
# This could be done with a list better later as long as dynamic allocation doesn't become a slowdown
# A list will reduce the number of opps needed to render everything out though
PixelsMarkedDirty = [ [ False for j in range(C_LIGHT_BOARD_HEIGHT) ] for i in range(C_LIGHT_BOARD_WIDTH) ]
PixelsDirty = False
WholeBufferDirty = False
lastTime = 0
fps = 0
framesSinceFullRefresh = 0

def display():
  global FrameBuffer
  global PixelsMarkedDirty
  global WholeBufferDirty
  global PixelsDirty
  global lastTime
  global fps
  global framesSinceFullRefresh

  deltaTime = time.perf_counter() - lastTime
  fps = 1.0 / deltaTime;
  print( "Frames Per Second = " + str( 1.0 / deltaTime) )
  lastTime = time.perf_counter()

  # If we did something to make all the pixels dirty write them all out and clean up the dirty markers along the way
  # OR if we need todo a sync frame for error eliminaton reasons
  if WholeBufferDirty == True or framesSinceFullRefresh > C_MAX_FRAMES_BETWEEN_FULL_SYNC:   
    for x in range(0, 5):
      for y in range(0, C_LIGHT_BOARD_HEIGHT):
        temp = FrameBuffer[x][y]
        writeXYRGBA( x, y, temp[C_BUFFER_RED], temp[C_BUFFER_GREEN], temp[C_BUFFER_BLUE], temp[C_BUFFER_BRIGHTNESS] )
        temp = FrameBuffer[x + 5][y]
        writeXYRGBA( x + 5, y, temp[C_BUFFER_RED], temp[C_BUFFER_GREEN], temp[C_BUFFER_BLUE], temp[C_BUFFER_BRIGHTNESS] )
        PixelsMarkedDirty[x][y] = False
        PixelsMarkedDirty[x + 5][y] = False
    PixelsDirty = False
    WholeBufferDirty = False
    framesSinceFullRefresh = 0
    return

  # if we have dirty pixels write them out to the board and clean up the dirty markers
  DirtyPixelCount = 0
  if PixelsDirty == True:
    for y in range(0, C_LIGHT_BOARD_HEIGHT):
      for x in range(0, C_LIGHT_BOARD_WIDTH):
        if PixelsMarkedDirty[x][y] == True:
          DirtyPixelCount += 1
          # for each pixel in matrix that is dirty write out the pixel
          # clear the dirty pixel array
          temp = FrameBuffer[x][y]
          writeXYRGBA( x, y, temp[C_BUFFER_RED], temp[C_BUFFER_GREEN], temp[C_BUFFER_BLUE], temp[C_BUFFER_BRIGHTNESS] )
          PixelsMarkedDirty[x][y] = False  
    PixelsDirty = False 

  # Keep track of how many frames we have had since a full refresh
  framesSinceFullRefresh += 1
  
  # if we reset all of the pixels this time keep track of that
  if( DirtyPixelCount >= C_NUMBER_OF_LIGHTS ):
    framesSinceFullRefresh = 0
  
def frameStabilizer():
  global fps
  global framesSinceFullRefresh 

  if( fps == 0 or framesSinceFullRefresh == 0 ):
     return
  
  if( fps - 1000 > 0):
    frameStabilizer.sleepTime += 0.1
  if( fps - 100 > 0 ):
    frameStabilizer.sleepTime += 0.01
#  if( fps - 45 > 0 ):
#    frameStabilizer.sleepTime += 0.001
#  if( fps < 15 ):
#    frameStabilizer.sleepTime

  if( fps > 24 ):
    frameStabilizer.sleepTime += 0.001
  elif( fps < 24 ): 
    frameStabilizer.sleepTime -= 0.001

  if( frameStabilizer.sleepTime > 0 ):
    time.sleep(frameStabilizer.sleepTime)
frameStabilizer.sleepTime = 0

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
  # Run each light 
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

def testBufferedWholeScreenTest():
  for r in range( 0, 15 ):
    for b in range( 0, 15 ):
      for g in range( 0, 15):
        bufferedWriteAllRGBA(r, g, b, 128)
        display()

def testBufferedWholeRGBScreenTest():
  test = 0
  while True:
    if test == 0:
      bufferedWriteAllRGBA( 15,  0,  0, 128)
      display()
      bufferedClearScreen()
      display()
      test = 1
    elif test == 1:
      bufferedWriteAllRGBA(  0, 15,  0, 128)
      display()
      bufferedClearScreen()
      display()
      test = 2
    else:
      bufferedWriteAllRGBA(  0,  0, 15, 128)
      display()
      bufferedClearScreen()
      display()
      test = 0

def TrippyHelp(x, y, test):
  if test == 0:
    bufferedWriteXYRGBA( x, y, 15,  0,  0, 128)
    test = 1
  elif test == 1:
    bufferedWriteXYRGBA( x, y, 0, 15,  0, 128)
    test = 2
  elif test == 2:
    bufferedWriteXYRGBA( x, y, 0,  0, 15, 128)
    test = 0
  elif test == 3:
    bufferedWriteXYRGBA( x, y, 0, 15, 15, 128)
    test = 4
  elif test == 4:
    bufferedWriteXYRGBA( x, y, 15, 15, 0, 128)
    test = 5
  elif test == 5:
    bufferedWriteXYRGBA( x, y, 15,  0, 15, 128)
    test = 0
  return test

def testTrippy():
  global WholeBufferDirty

  temp = 0
  while True:
    for x in range(0, C_LIGHT_BOARD_WIDTH):
      for y in range(0, C_LIGHT_BOARD_HEIGHT):
        temp = TrippyHelp(x, y, temp)
    WholeBufferDirty = True
    display()
  
def testBufferedRThenBThenG():
  while True:
    for x in range(0, C_LIGHT_BOARD_WIDTH):
      for y in range(0, C_LIGHT_BOARD_HEIGHT):
        for colorSelect in range(0,3):
          for amount in range(0, 16):
            if colorSelect == 0:
              bufferedWriteXYRGBA(x, y, amount, 0, 0)
            elif colorSelect == 1:
              bufferedWriteXYRGBA(x, y, 0, amount, 0)
            else:
              bufferedWriteXYRGBA(x, y, 0, 0, amount)
            display()
            frameStabilizer()
            
