import serial

# Throw down some constants
C_BAUDRATE = 31000
C_NUMBER_OF_LIGHTS = 70

# Setup and open the Serial connection
ser = serial.Serial()
ser.baudrate = C_BAUDRATE
ser.port = "//dev/ttyACM0"
ser.open()

def write( led, color1, color2 ):
  ser.write( [ led, 0xFF, color1, color2 ] )
  
def writeAll( color1, color2 ):
  for led in range(0, C_NUMBER_OF_LIGHTS):
    writeout( led, color1, color2 )
    
def test()
  for x in range( 0, 255 ):
    for y in range( 0, 16 ):
      writeAll( y, x )