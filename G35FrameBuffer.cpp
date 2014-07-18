// Hack Pittsburgh Frame Buffer Test
#include <Arduino.h>
#include <stdint.h>
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
const uint8_t MESSAGE_SIZE            = 4;                                      // Number of bytes in message
const int FULL_BUFFER_OF_MESSAGES = NUMBER_OF_LEDS;                         // Max Messages we could get in one frame buffer update
const int MESSAGE_BUFFER_SIZE     = MESSAGE_SIZE * FULL_BUFFER_OF_MESSAGES; // Size of buffer needed to store messages
const uint8_t MESSAGE_BYTE_ID              = 0;
const uint8_t MESSAGE_BYTE_BRIGHTNESS      = 1;
const uint8_t MESSAGE_BYTE_COLOR_GREEN_RED = 2;
const uint8_t MESSAGE_BYTE_COLOR_BLUE      = 3;

// Message Buffer and its data
uint8_t MessageBuffer[MESSAGE_BUFFER_SIZE] = {0};
uint16_t FirstOpenByteInBuffer             =  0;
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
uint8_t StringOneMessage[MESSAGE_SIZE] ={0};
uint8_t StringTwoMessage[MESSAGE_SIZE] ={0};

void setup()
{  
  // Constants For Serial Connection
  //const int BAUD_RATE = 9600;
  //const int BAUD_RATE = 19200;
  const int BAUD_RATE = 28800; // Best Working so far still testing
  //const int BAUD_RATE = 31000;   // ?? Working Provides Faster framerates
  //const int BAUD_RATE = 32700;
  //const int BAUD_RATE = 32750;  // 11.67 frames a second
  //Not Working////////////////////////////////////////
  //const int BAUD_RATE = 32790;
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
  
  // Disable Timer started by G35String
  // WARNING: TIMER Zero IS USED FOR deleyMillisecond command
  // Doing this probably borks it but i dont care.
  TCCR0B &= ~(1 << CS00 | 1 << CS01 | 1 << CS02);
  
  // Start Serial Connection 
  Serial.begin(BAUD_RATE);
  
  // Setup Timer One
  SetupTimerOne();
  //startTimer();
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
    if( MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_ID] < LEDS_PER_STRING )
    {
      // Prepare messages for bit bang
      if( StringOneState == NOT_TRANSMITTING )
      { 
	// Grab the message to send and place it in memory for the interupt
        StringOneMessage[MESSAGE_BYTE_ID]              = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_ID];
	StringOneMessage[MESSAGE_BYTE_BRIGHTNESS]      = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_BRIGHTNESS];
	StringOneMessage[MESSAGE_BYTE_COLOR_GREEN_RED] = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_COLOR_GREEN_RED];
        StringOneMessage[MESSAGE_BYTE_COLOR_BLUE]      = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_COLOR_BLUE];
	
	// Set initial ready pulse
        StringOneNextBit = SEND_A_ONE;
	
	// Set state machine to READY_TO_TRANSMIT
        StringOneState   = READY_TO_TRANSMIT;
        
	// Make sure the timer is running
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
	// Grab the message to send and place it in memory for the interupt
        StringTwoMessage[MESSAGE_BYTE_ID]              = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_ID] - LEDS_PER_STRING;
	StringTwoMessage[MESSAGE_BYTE_BRIGHTNESS]      = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_BRIGHTNESS];
	StringTwoMessage[MESSAGE_BYTE_COLOR_GREEN_RED] = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_COLOR_GREEN_RED];
	StringTwoMessage[MESSAGE_BYTE_COLOR_BLUE]      = MessageBuffer[ FirstMessageToProcess + MESSAGE_BYTE_COLOR_BLUE];
	
	// Set initial ready pulse
        StringTwoNextBit = SEND_A_ONE;
	
	// Set state machine to READY_TO_TRANSMIT
        StringTwoState   = READY_TO_TRANSMIT;

	// Make sure the timer is running
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
    Serial.print("First Message to Process = "); Serial.println(FirstMessageToProcess);
    Serial.print("Last Message to Process = ");  Serial.println(LastMessageToProcess);
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
      if( stringOneProgress < 6 )
      {
        if( (StringOneMessage[MESSAGE_BYTE_ID] & 0x20 ) != 0 )
        { 
          StringOneNextBit = SEND_A_ONE;
        }
        else
        {
          StringOneNextBit = SEND_A_ZERO;
        }
        StringOneMessage[MESSAGE_BYTE_ID] = StringOneMessage[MESSAGE_BYTE_ID] << 1;
      }
      else if( stringOneProgress < 14 )
      {
        if( (StringOneMessage[MESSAGE_BYTE_BRIGHTNESS] & 0x80 ) != 0 )
        { 
          StringOneNextBit = SEND_A_ONE;
        }
        else
        {
          StringOneNextBit = SEND_A_ZERO;
        }
        StringOneMessage[MESSAGE_BYTE_BRIGHTNESS] = StringOneMessage[MESSAGE_BYTE_BRIGHTNESS] << 1;
      }
      else if( stringOneProgress < 18 )
      {
        if( (StringOneMessage[MESSAGE_BYTE_COLOR_BLUE] & 0x08 ) != 0 )
        { 
          StringOneNextBit = SEND_A_ONE;
        }
        else
        {
          StringOneNextBit = SEND_A_ZERO;
        }
        StringOneMessage[MESSAGE_BYTE_COLOR_BLUE] = StringOneMessage[MESSAGE_BYTE_COLOR_BLUE] << 1;
      }
      else
      {
        if( (StringOneMessage[MESSAGE_BYTE_COLOR_GREEN_RED] & 0x80 ) != 0 )
        { 
          StringOneNextBit = SEND_A_ONE;
        }
        else
        {
          StringOneNextBit = SEND_A_ZERO;
        }
        StringOneMessage[MESSAGE_BYTE_COLOR_GREEN_RED] = StringOneMessage[MESSAGE_BYTE_COLOR_GREEN_RED] << 1;
        if(stringOneProgress > 25) StringOneState = SENDING_STOP_BIT;	
      }
      stringOneProgress++;
      
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
      if( stringTwoProgress < 6 )
      {
        if( (StringTwoMessage[MESSAGE_BYTE_ID] & 0x20 ) != 0 )
        { 
          StringTwoNextBit = SEND_A_ONE;
        }
        else
        {
          StringTwoNextBit = SEND_A_ZERO;
        }
        StringTwoMessage[MESSAGE_BYTE_ID] = StringTwoMessage[MESSAGE_BYTE_ID] << 1;
      }
      else if( stringTwoProgress < 14 )
      {
        if( (StringTwoMessage[MESSAGE_BYTE_BRIGHTNESS] & 0x80 ) != 0 )
        { 
          StringTwoNextBit = SEND_A_ONE;
        }
        else
        {
          StringTwoNextBit = SEND_A_ZERO;
        }
        StringTwoMessage[MESSAGE_BYTE_BRIGHTNESS] = StringTwoMessage[MESSAGE_BYTE_BRIGHTNESS] << 1;
      }
      else if( stringTwoProgress < 18 )
      {
        if( (StringTwoMessage[MESSAGE_BYTE_COLOR_BLUE] & 0x08 ) != 0 )
        { 
          StringTwoNextBit = SEND_A_ONE;
        }
        else
        {
          StringTwoNextBit = SEND_A_ZERO;
        }
        StringTwoMessage[MESSAGE_BYTE_COLOR_BLUE] = StringTwoMessage[MESSAGE_BYTE_COLOR_BLUE] << 1;

      }        
      else
      {
        if( (StringTwoMessage[MESSAGE_BYTE_COLOR_GREEN_RED] & 0x80 ) != 0 )
        { 
          StringTwoNextBit = SEND_A_ONE;
        }
        else
        {
          StringTwoNextBit = SEND_A_ZERO;
        }
        StringTwoMessage[MESSAGE_BYTE_COLOR_GREEN_RED] = StringTwoMessage[MESSAGE_BYTE_COLOR_GREEN_RED] << 1;
            if(stringTwoProgress > 25) StringTwoState = SENDING_STOP_BIT; 
      }

      stringTwoProgress++;
      
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
