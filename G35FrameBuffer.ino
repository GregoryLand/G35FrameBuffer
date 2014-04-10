// Hack Pittsburgh Frame Buffer Test
#include <stdint.h>
//#include <SoftwareSerial.h>
#include <G35String.h>
#include <G35StringGroup.h>
//#include <TimerOne.h>
//#include "led_utils.h"

// Debug constants
//#define DEBUG_SERIAL_MESSAGE_BUFFER
//#define DEBUG_COLOR_MESSAGES
//#define DEBUG_SERIAL_LOOPCOUNT
// Create the SoftwareSerial object and give it a rx and tx pin
const int RX_PIN = 2;
const int TX_PIN = 3;
//SoftwareSerial LCD(RX_PIN, TX_PIN); 

// Constants for strings and light count.
const int LEDS_PER_STRING   = 35;
const int NUMBER_OF_STRINGS =  2;
const int NUMBER_OF_LEDS = NUMBER_OF_STRINGS * LEDS_PER_STRING;  // if number of leds is larger then 255 we need to modify the type for FirstMessageToProcess and LastMessageToProcess
const int STRING_ONE_PIN = 8;
const int STRING_TWO_PIN = 7;

// Build Objects for each string
G35String string1(STRING_ONE_PIN, LEDS_PER_STRING);
G35String string2(STRING_TWO_PIN, LEDS_PER_STRING);

// Setup struct for messages
struct Message
{
  uint8_t    led;
  uint8_t    brightness;
  uint16_t   color;
};

// Constants Buffer for messages
const int MESSAGE_SIZE            = 4;                                      // Number of bytes in message
const int FULL_BUFFER_OF_MESSAGES = NUMBER_OF_LEDS;                         // Max Messages we could get in one frame buffer update
const int MESSAGE_BUFFER_SIZE     = MESSAGE_SIZE * FULL_BUFFER_OF_MESSAGES; // Size of buffer needed to store messages

// Message Buffer and its data
byte MessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
int FirstOpenByteInBuffer       =  0;
int8_t FirstMessageToProcess    =  0;
int8_t LastMessageToProcess     = -1;
uint8_t BytesOfNextMessageSoFar =  0;

// Data for Bit Bang 
// States
const uint8_t NOT_TRANSMITTING   = 0;
const uint8_t READY_TO_TRANSMIT  = 1;
const uint8_t SENDING_START_BIT  = 2;
const uint8_t SENDING_DATA       = 3;
const uint8_t SENDING_STOP_BIT   = 4;

uint8_t StringOneState        = NOT_TRANSMITTING;
uint8_t StringTwoState        = NOT_TRANSMITTING;

uint8_t StringOneSendProgress = 0;
uint8_t StringTwoSendProgress = 0;

uint32_t StringOneMessage = 0;
uint32_t StringTwoMessage = 0;

void setup()
{  
  // Constants For LCD SCREEN
  const int LCD_BAUD_RATE        = 9600;
  const int FORM_FEED            = 12;
  const int BACKLIGHT_ON         = 17;
  const int MOVE_TO_LINE_1_POS_0 = 148;
  
  // Constants For Serial Connection
  const int BAUD_RATE            = 9600;//115200; //38400; // Basic tests gives 31000 as a close to max baud rate. more testing needed 

  // Clear and Write to Lcd Screenx
  //LCD.begin(LCD_BAUD_RATE);              // Setup Lcd baud rate
  //LCD.write(FORM_FEED);                  // Form Feed, clear screen
  //LCD.write(BACKLIGHT_ON);               // Turn on backlight (Parallax LCD)
  //LCD.print("Framebuffer Test");         // Pass a message to display
  //LCD.write(MOVE_TO_LINE_1_POS_0);       // move to line 1 pos 0

  // Start Serial Connection 
  Serial.begin(BAUD_RATE);
  
  // Start up the G35
  string1.enumerate();
  string2.enumerate();
}

void loop() 
{  
  // If Messages To Process
  if( LastMessageToProcess != -1 )
  {
    #ifdef DEBUG_COLOR_MESSAGES
    Serial.print("First Message to Process = "); Serial.println(FirstMessageToProcess);
    Serial.print("Last Message to Process = ");  Serial.println(LastMessageToProcess);
    #endif

    // Send Message
    if( reinterpret_cast<Message&>(MessageBuffer[FirstMessageToProcess * MESSAGE_SIZE]).led < LEDS_PER_STRING )
    {
      // Prepare messages for bit bang
      if( StringOneState == NOT_TRANSMITTING )
      { 
        StringOneMessage = PackG35Message( (reinterpret_cast<Message&>(MessageBuffer[FirstMessageToProcess * MESSAGE_SIZE])) );
        StringOneState = READY_TO_TRANSMIT;
      }
    }
    else
    {
      if( StringTwoState == NOT_TRANSMITTING ) 
      {
        StringTwoMessage = PackG35Message( (reinterpret_cast<Message&>(MessageBuffer[FirstMessageToProcess * MESSAGE_SIZE])) );
        StringTwoState = READY_TO_TRANSMIT;
      }
      
    }
    // Setup for next run
    // If We  have processed all of the messages set state to show it
    if( FirstMessageToProcess == LastMessageToProcess )
    {
      LastMessageToProcess  = -1;
      FirstMessageToProcess =  0;
    }
    else
    {
      // Move forward in the buffer
      FirstMessageToProcess++;
    }
    Serial.print(" Packed Message: "); Serial.println(StringOneMessage, BIN);
    Serial.print(" Packed Message: "); Serial.println(StringTwoMessage, BIN);
    #ifdef DEBUG_COLOR_MESSAGES
    Serial.print("First Message to Process = "); Serial.println(FirstMessageToProcess);
    Serial.print("Last Message to Process = ");  Serial.println(LastMessageToProcess);
    Serial.flush();
    #endif
  }
}

void serialEvent()
{
  #ifdef DEBUG_SERIAL_LOOPCOUNT
  // Keep track of number of loops for debug reasons only
  int loopCounter = 0;
  #endif
  
  // If no activity reset buffer
  if( BytesOfNextMessageSoFar == 0 && FirstMessageToProcess == 0 && LastMessageToProcess == -1)
  {
    FirstOpenByteInBuffer = 0;
  }
  
  // When we are informed that there is serial data waiting for us in the buffer
  // then empty out the ring buffer for the serial port
  // ReadBuffer just calls read in a loop.
  // ARG NO WAY TO COPY WHOLE BUFFER AT ONCE!!! HATE
  while( Serial.available() )
  { 
    #ifdef DEBUG_SERIAL_LOOPCOUNT
    loopCounter++;
    #endif
        
    // Using % op is stupidly inefficent on avr here so just reset the counter
    if( FirstOpenByteInBuffer >= MESSAGE_BUFFER_SIZE )
    {
      #ifdef DEBUG_SERIAL_MESSAGE_BUFFER
      // If we are about to start the buffer over and we haven't processed the first message
      // in the buffer yet we are in trouble
      if( FirstMessageToProcess == 0 && LastMessageToProcess != 0 ) Serial.write("ERROR: Message Buffer was overwritten");
      #endif
      
      FirstOpenByteInBuffer = 0;
    }
    
    // Write the byte to the message buffer
    MessageBuffer[ FirstOpenByteInBuffer] = Serial.read();
    
    // Switch next active byte
    FirstOpenByteInBuffer++;
    
    // Keep track of how many bytes belong in the message we are saving
    BytesOfNextMessageSoFar++;
    
    // We have a complete message so change message counters
    if( BytesOfNextMessageSoFar == MESSAGE_SIZE )
    {      
      // Reset counter
      BytesOfNextMessageSoFar = 0;
      
      // Change Message Counter
      LastMessageToProcess++;
      
      // If we have filled the buffer before emptying it we are screwed
      #ifdef DEBUG_SERIAL_MESSAGE_BUFFER
      if( LastMessageToProcess >= FULL_BUFFER_OF_MESSAGES && FirstMessageToProcess == 0 ) Serial.write("ERROR: Message Buffer is full");
      #endif
      
      // If we hit the end of the buffer start over at the front
      if( LastMessageToProcess >= FULL_BUFFER_OF_MESSAGES ) LastMessageToProcess = 0;
    }
  }
  
  #ifdef DEBUG_SERIAL_LOOPCOUNT
  const int ASSUMED_NUMBER_OF_MAX_LOOPS = 64;
  if( loopCounter > ASSUMED_NUMBER_OF_MAX_LOOPS ) Serial.write("We are pulling more data out of the ring buffer then I thought possible");
  #endif
}

inline void BitBangString( uint8_t stringNumber, uint8_t state, uint8_t progress, uint32_t data )
{ 
  const int SHORT_PULSE = 0;
  const int LONG_PULSE  = 0;
  
  switch(state)
  {
    case NOT_TRANSMITTING:
      return;
    case READY_TO_TRANSMIT:
      break;
    case SENDING_START_BIT:
      // Shift <-- this way and send the bit
      break;
    case SENDING_DATA:
      break;
    case SENDING_STOP_BIT:
      break;
  }
}
void BitBangStringOne()
{
  BitBangString(STRING_ONE_PIN, StringOneState, StringOneSendProgress, StringOneMessage );
}
void BitBangStringTwo()
{
  BitBangString(STRING_TWO_PIN, StringTwoState, StringTwoSendProgress, StringTwoMessage );
}

uint32_t PackG35Message( struct Message& data )//( Message& data )
{
  // Constants
  const uint8_t LAST_BIT_PLACE = 26;
  
  uint32_t temp = 0;
  // Pack the data struct
  temp += data.color;
  // Pack Brightness bits into bits 21-13
  //temp = temp | (data.brightness << 12);
  // Pack ID Number
  temp += (data.led << 21);
  
  return temp;
}
