// Hack Pittsburgh Frame Buffer Test
#include <Arduino.h>
#include <stdint.h>
#include <util/atomic.h>
#include <G35String.h>
#include "G35TimerOne.h"

// Debug constants
// WARNING: Debuging outputs WILL screw with the timing for the interupts
// You may need to adjust the interupt timer if attempting to debug some
// parts of the code
//#define DEBUG_SERIAL_MESSAGE_BUFFER
//#define DEBUG_COLOR_MESSAGES
//#define DEBUG_SERIAL_LOOPCOUNT
//#define DEBUG_BIT_BANG
//#define LCD_SCREEN_ENABLE

#ifdef LCD_SCREEN_ENABLE
  #include <SoftwareSerial.h>
  
  // Create the SoftwareSerial object and give it a rx and tx pin
  const int RX_PIN = 2;
  const int TX_PIN = 3;
  SoftwareSerial LCD(RX_PIN, TX_PIN);   
#endif

// Constants for strings and light count.
const int LEDS_PER_STRING   = 35;
const int NUMBER_OF_STRINGS =  2;
const int NUMBER_OF_LEDS = NUMBER_OF_STRINGS * LEDS_PER_STRING;  // if number of leds is larger then 255 we need to modify the type for FirstMessageToProcess and LastMessageToProcess
const int STRING_ONE_PIN = 10;
const int STRING_TWO_PIN = 9;

// Build Objects for each string
G35String string1(STRING_ONE_PIN, LEDS_PER_STRING);
G35String string2(STRING_TWO_PIN, LEDS_PER_STRING);

// Constants Buffer for messagestest
const int MESSAGE_SIZE            = 4;                                      // Number of bytes in message
const int FULL_BUFFER_OF_MESSAGES = NUMBER_OF_LEDS;                         // Max Messages we could get in one frame buffer update
const int MESSAGE_BUFFER_SIZE     = MESSAGE_SIZE * FULL_BUFFER_OF_MESSAGES; // Size of buffer needed to store messages

// Setup struct for messages
struct Message
{
  uint8_t    led;
  uint8_t    brightness;
  uint16_t   color;
};

// Message Buffer and its data
uint8_t MessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
int16_t FirstOpenByteInBuffer   =  0;
int8_t FirstMessageToProcess    =  0;
int8_t LastMessageToProcess     = -1;
uint8_t BytesOfNextMessageSoFar =  0;

// Data for Bit Bang 
// States for bit bang
const uint8_t NOT_TRANSMITTING   = 0;
const uint8_t READY_TO_TRANSMIT  = 1;
const uint8_t SENDING_DATA       = 2;
const uint8_t SENDING_STOP_BIT   = 3;
const uint8_t DELAY_BEFORE_READY = 4;
uint8_t StringOneState = NOT_TRANSMITTING;
uint8_t StringTwoState = NOT_TRANSMITTING;
uint16_t StringOneNextBit = HOLD_LOW;
uint16_t StringTwoNextBit = HOLD_LOW;
uint32_t StringOneMessage = 0;
uint32_t StringTwoMessage = 0;

uint32_t PackG35Message( uint8_t led, uint8_t brightness, uint16_t color )
{
  // Do the bit packing  
  uint32_t temp = 0;
  // Pack the data struct
  temp += color;
  // Pack Brightness bits into bits 21-13
  temp += (uint32_t)brightness << 12;
  // Pack ID Number
  temp += (uint32_t)led << 20;
  
  return temp;
}

void setup()
{  
  // Constants For Serial Connection
  //const int BAUD_RATE = 9600;
  //const int BAUD_RATE = 19200;
  //const int BAUD_RATE = 28800; // Best Working so far still testing
  //const int BAUD_RATE = 31000;   // ?? Working Provides Faster framerates
  //const int BAUD_RATE = 32700;
  //const int BAUD_RATE = 32000;
  const int BAUD_RATE = 32750;  // 11.67 frames a second
  //Not Working////////////////////////////////////////
  //const int BAUD_RATE = 38400;
  //const int BAUD_RATE = 57600;
  //const int BAUD_RATE = 115200;

  // Constants For LCD SCREEN
  #ifdef LCD_SCREEN_ENABLE
    const int LCD_BAUD_RATE        = 9600;
    const int FORM_FEED            = 12;
    const int BACKLIGHT_ON         = 17;
    const int MOVE_TO_LINE_1_POS_0 = 148;
  
    // Clear and Write to Lcd Screenx
    LCD.begin(LCD_BAUD_RATE);              // Setup Lcd baud rate
    LCD.write(FORM_FEED);                  // Form Feed, clear screen
    LCD.write(BACKLIGHT_ON);               // Turn on backlight (Parallax LCD)
    LCD.print("Framebuffer Test");         // Pass a message to display
    LCD.write(MOVE_TO_LINE_1_POS_0);       // move to line 1 pos 0
  #endif
    
  // Start up the G35
  string1.enumerate();
  string2.enumerate();
  
  // Clear Red setup color
  string1.fill_color( 0, LEDS_PER_STRING, 0, 0);
  string2.fill_color( 0, LEDS_PER_STRING, 0, 0);

  // Start Serial Connection 
  Serial.begin(BAUD_RATE);
  
  SetupTimerOne();
}

void loop() 
{  
  #ifdef DEBUG_BIT_BANG
    Serial.print("Line1 Send Progress = "); Serial.print(StringOneSendProgress);
  #endif  
  #ifdef DEBUG_COLOR_MESSAGES
    Serial.print(" Packed Message: "); Serial.println(StringOneMessage, BIN);
    Serial.print(" Packed Message: "); Serial.println(StringTwoMessage, BIN);
  #endif  

  // If Messages To Process
  if( LastMessageToProcess != -1 )
  {
    #ifdef DEBUG_COLOR_MESSAGES
      Serial.print("First Message to Process = "); Serial.println(FirstMessageToProcess);
      Serial.print("Last Message to Process = ");  Serial.println(LastMessageToProcess);
    #endif

    // Send Message
    Message tempM = reinterpret_cast<Message&>( (MessageBuffer[FirstMessageToProcess * MESSAGE_SIZE]) );
      
    if( tempM.led < LEDS_PER_STRING )
    {
      // Prepare messages for bit bang
      if( StringOneState == NOT_TRANSMITTING )
      { 
//	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
//	{
          StringOneMessage = PackG35Message(tempM.led, tempM.brightness, tempM.color);
	  StringOneNextBit = SEND_A_ONE;
//	}
        StringOneState   = READY_TO_TRANSMIT;
        if( StringTwoState == NOT_TRANSMITTING ) startTimer();
        #ifdef DEBUG_COLOR_MESSAGES
          Serial.println("Line1");
        #endif
      }
    }
    else
    {
      if( StringTwoState == NOT_TRANSMITTING ) 
      {
//	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
//	{
          StringTwoMessage = PackG35Message(tempM.led - LEDS_PER_STRING, tempM.brightness, tempM.color);
	  StringTwoNextBit = SEND_A_ONE;
//	}
        StringTwoState   = READY_TO_TRANSMIT;
        if( StringOneState == NOT_TRANSMITTING ) startTimer();
        #ifdef DEBUG_COLOR_MESSAGES
          Serial.println("Line2");
        #endif
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
    
    #ifdef DEBUG_COLOR_MESSAGES
    Serial.print(" Packed Message: "); Serial.println(StringOneMessage, BIN);
    Serial.print(" Packed Message: "); Serial.println(StringTwoMessage, BIN);
    Serial.print("First Message to Process = "); Serial.println(FirstMessageToProcess);
    Serial.print("Last Message to Process = ");  Serial.println(LastMessageToProcess);
    Serial.flush();
    #endif
  }

}

// This is our callback from the timer when top is reached
// This meens 30 microseconds has passed and we are ready to
// send the next bit
uint8_t stringOneProgress = 0;
uint8_t stringTwoProgress = 0;
ISR( TIMER1_OVF_vect )
{
  OCR1B = StringOneNextBit;
  OCR1A = StringTwoNextBit;
  
  switch(StringOneState)
  {
    case READY_TO_TRANSMIT:
      StringOneState = SENDING_DATA;
    case SENDING_DATA:
      if( (StringOneMessage & ( 1L << 25 )) > 0 )
      { 
        StringOneNextBit = SEND_A_ONE;
      }
      else
      {
        StringOneNextBit = SEND_A_ZERO;
      }
      stringOneProgress++;
      StringOneMessage = StringOneMessage << 1;
      if(stringOneProgress > 25) StringOneState = SENDING_STOP_BIT;
      break;
    case SENDING_STOP_BIT:
      StringOneNextBit = HOLD_LOW;
      StringOneState   = NOT_TRANSMITTING;
      stringOneProgress = 0;
      break;
    case NOT_TRANSMITTING:
      break;
  }
  switch(StringTwoState)
  {
    case READY_TO_TRANSMIT:
      StringTwoState = SENDING_DATA;
    case SENDING_DATA:
      if( (StringTwoMessage & ( 1L << 25 )) > 0 )
      { 
        StringTwoNextBit = SEND_A_ONE;
      }
      else
      {
        StringTwoNextBit = SEND_A_ZERO;
      }
      stringTwoProgress++;
      if(stringTwoProgress > 25) StringTwoState = SENDING_STOP_BIT;
      StringTwoMessage = StringTwoMessage << 1;
      break;
    case SENDING_STOP_BIT:
      StringTwoNextBit = HOLD_LOW;
      StringTwoState   = NOT_TRANSMITTING;
      stringTwoProgress = 0;
      break;
    case NOT_TRANSMITTING:
      break;
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
//    ATOMIC_BLOCK( ATOMIC_RESTORESTATE )
//    {
      FirstOpenByteInBuffer++;
//    }
    
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
