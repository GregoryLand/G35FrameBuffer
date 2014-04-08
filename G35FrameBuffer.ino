// Hack Pittsburgh Frame Buffer Test
#include <stdint.h>
#include <SoftwareSerial.h>
#include <G35String.h>
#include <G35StringGroup.h>
#include "led_utils.h"



// Global Items That need to be setup in Setup() and used in loop()
// Doing it this way so I am forced to pass things into functions
// Instead of passing it around globals all over the place.
struct ProgramState
{
  SoftwareSerial& LCD;
};
ProgramState* state;

// Constants for string and light count.  
const int NUMBER_OF_STRINGS = 2;
const int NUMBER_OF_LEDS = NUMBER_OF_STRINGS * LEDS_PER_STRING;

void setup() 
{
  // Constants For LCD SCREEN
  const int RX_PIN = 2;
  const int TX_PIN = 3;
  const int LCD_BAUD_RATE = 9600;
  const int FORM_FEED = 12;
  const int BACKLIGHT_ON = 17;
  const int MOVE_TO_LINE_1_POS_0 = 148;
  
  // Constants For Serial Connection
  const int BAUD_RATE = 9600;

  // Start Serial Connection
  Serial.begin(BAUD_RATE);

  // Clear and Write to Lcd Screen
  state->LCD = SoftwareSerial(RX_PIN, TX_PIN);  // Create the SoftwareSerial object and give it a rx and tx pin
  state->LCD.begin(LCD_BAUD_RATE);              // Setup Lcd baud rate
  state->LCD.write(FORM_FEED);                  // Form Feed, clear screen
  state->LCD.write(BACKLIGHT_ON);               // Turn on backlight (Parallax LCD)
  state->LCD.print("Frame Buffer Test");        // Pass a message to display
  state->LCD.write(MOVE_TO_LINE_1_POS_0);       // move to line 1 pos 0
  
  // Start up the G35
  initializeLedBoard();
}

void loop() 
{
}
