// Hack Pittsburgh Frame Buffer Test
#include <stdint.h>
//#include <SoftwareSerial.h>
#include <G35String.h>
#include <G35StringGroup.h>
#include "led_utils.h"

// Create the SoftwareSerial object and give it a rx and tx pin
const int RX_PIN = 2;
const int TX_PIN = 3;
//SoftwareSerial LCD(RX_PIN, TX_PIN); 

// Constants for string and light count.  
const int NUMBER_OF_STRINGS = 2;
const int NUMBER_OF_LEDS = NUMBER_OF_STRINGS * LEDS_PER_STRING;

// Setup struct for messages
struct Message
{
  byte     led;
  byte     brightness;
  uint16_t color;
};

void setup() 
{
  // Constants For LCD SCREEN
  const int LCD_BAUD_RATE = 9600;
  const int FORM_FEED = 12;
  const int BACKLIGHT_ON = 17;
  const int MOVE_TO_LINE_1_POS_0 = 148;
  
  // Constants For Serial Connection
  const int BAUD_RATE = 31000; // Basic tests gives 31000 as a close to max baud rate. more testing needed 

  // Start Serial Connection
  Serial.begin(BAUD_RATE);

  // Clear and Write to Lcd Screen
  //LCD.begin(LCD_BAUD_RATE);              // Setup Lcd baud rate
  //LCD.write(FORM_FEED);                  // Form Feed, clear screen
  //LCD.write(BACKLIGHT_ON);               // Turn on backlight (Parallax LCD)
  //LCD.print("Framebuffer Test");        // Pass a message to display
  //LCD.write(MOVE_TO_LINE_1_POS_0);       // move to line 1 pos 0
  
  // Start up the G35
  initializeLedBoard();
  clearscreen();
}

void loop() 
{
  const size_t MESSAGE_SIZE = 4;
  char message[MESSAGE_SIZE] = {};
  
  //while ( Serial.available() )
  //{ 
    // Read in the bytes
    Serial.readBytes(message, MESSAGE_SIZE);
    
    // Overlay a struct on the data
    Message* temp = (Message*)&message;
    
    // Set the led
    setLed(temp->led, temp->brightness, temp->color);
  //}
}

/*void serialEvent()
{
  const size_t MESSAGE_SIZE = 4;
  char message[MESSAGE_SIZE] = {};
  
  //while ( Serial.available() )
  //{ 
    // Read in the bytes
    Serial.readBytes(message, MESSAGE_SIZE);
    
    // Overlay a struct on the data
    Message* temp = (Message*)&message;
    
    // Set the led
    setLed(temp->led, temp->brightness, temp->color);
  //}
}*/
// Format
// ser.write( [ led, bright, color1 , color2 ] )
