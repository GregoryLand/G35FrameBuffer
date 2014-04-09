#!/usr/bin/python
# Output: Bit pattern for bite stream
# | led | brightness | color1 | color2 |
# led is any number between 0 inclusive and C_NUMBER_OF_LIGHTS exclusive
# brightness is any number between 0 and 255
# color1 is the the packed red and green values the 4 high order bits are green and the 4 low order bits are red
# color2 is just the 4 low order blue bits
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

###### Imports ######
import serial

###### Constants(sort of dont change while running code) #####
# Throw down some constants
C_BAUDRATE = 31000
C_NUMBER_OF_LIGHTS = 70
C_LOOKUP_TABLE = [[63,64,65,66,67,68,69],[62,61,60,59,58,57,56],[49,50,51,52,53,54,55],[48,47,46,45,44,43,42],[35,36,37,38,39,40,41],[34,33,32,31,30,29,28],[21,22,23,24,25,26,27],[20,19,18,17,16,15,14],[7,8,9,10,11,12,13],[6,5,4,3,2,1,0]]                         
C_LIGHT_BOARD_HEIGHT = 7
C_LIGHT_BOARD_WIDTH  = 10

###### Flags ########
F_DEBUG = False

# Setup and open the Serial connection
ser = serial.Serial()
ser.baudrate = C_BAUDRATE
ser.port = "//dev/ttyACM1"
ser.open()

############# Low Level Functions ###################
# These have no error checking you have been warned
#####################################################
def write( led, color1, color2 ):
  ser.write( [ led, 0xFF, color1, color2 ] )
  if F_DEBUG == True:
    print( "led = " + str(led) + " brightness = 0xFF" + " color1 = " + str(color1) + " color2 = " + str(color2) )
  
def writeRGBA( led, red, blue, green, brightness ):
  green = green << 4
  ser.write( [ led, brightness, green + red, blue ] )
  if F_DEBUG == True:
    print( "led = " + str(led) + " red = " + str(red) + " blue = " + str(blue) + " green = " + str(green) )
    print( "Sending: led = " +  str(led) + " brightness = " + str(brightness) + " color1 = " + str(green + red) + " color2 = " + str(blue) )

def writeAll( color1, color2 ):
  for led in range(0, C_NUMBER_OF_LIGHTS):
    write( led, color1, color2 )

def writeAllRGBA( red, blue, green, brightness ):
  for led in range(0, C_NUMBER_OF_LIGHTS):
    writeRGBA( led, red, blue, green, brightness )
      
def clearscreen():
  writeAllRGBA(0, 0, 0, 0);

############ Turn the screen into a grid ##################
#
###########################################################
writeXYRGBA( xCord, yCord, red, blue, green, brightness ):
  writeRGBA( C_LOOKUP_TABLE[xCord][yCord], red, blue, green, brightness )
  


def test():
  for r in range( 0, 15 ):
    for b in range( 0, 15 ):
      for g in range( 0, 15):
        writeAllRGBA(r, g, b, 128)
	