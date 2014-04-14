
#ifndef LED_UTILS_H
#define LED_UTILS_H

/*************************************************************************************
 * Board Pattern 
 * 
 * |----|----|----|----|----|----|----|----|----|----|
 * | 69 | 56 | 55 | 42 | 41 | 28 | 27 | 14 | 13 |  0 |
 * | 68 | 57 | 54 | 43 | 40 | 29 | 26 | 15 | 12 |  1 |
 * | 67 | 58 | 53 | 44 | 39 | 30 | 25 | 16 | 11 |  2 |
 * | 66 | 59 | 52 | 45 | 38 | 31 | 24 | 17 | 10 |  3 |
 * | 65 | 60 | 51 | 46 | 37 | 32 | 23 | 18 |  9 |  4 |
 * | 64 | 61 | 50 | 47 | 36 | 33 | 22 | 19 |  8 |  5 |
 * | 63 | 62 | 49 | 48 | 35 | 34 | 21 | 20 |  7 |  6 |
 * |----|----|----|----|----|----|----|----|----|----|
 * 
 * Window Pattern From Outside
 * 
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * |    |    |    |    |    |    |    |
 * |----|----|----|----|----|----|----|
 * 
 ***********************************************************************************/
#include <Arduino.h>

#include <G35String.h>
#include <G35StringGroup.h>

#include "fonts.h"

#define  LED_STRING_1  9
#define  LED_STRING_2 10
#define  LEDS_PER_STRING  35


G35String lights_1(LED_STRING_1, LEDS_PER_STRING);
G35String lights_2(LED_STRING_2, LEDS_PER_STRING);
G35StringGroup string_group;

void initializeLedBoard ( void )
{
  lights_1.enumerate();
  lights_2.enumerate();

  string_group.AddString(&lights_1);
  string_group.AddString(&lights_2);

}

void setCol(byte col, byte pattern, byte intesity, unsigned int color) 
{
  byte led;
  byte dir = 0;
  byte pattern_mask = 0x40;

  if (col & 0x01) dir = 6;

  for ( byte i = 0; i < 7; i++)
  {  
    led = (col*7)+(abs(dir-i)); 
    if (pattern & pattern_mask) 
    { 
      string_group.set_color(led, intesity, color);
    }
    else
    { 
      string_group.set_color(led, intesity, COLOR_BLACK);
    }
    pattern_mask = pattern_mask >> 1;
  }
}


void scrollSequence(char *seq, byte len, byte intensity, unsigned int color) 
{
  // 10 is the number of columns in the grid
  for (int p = 0; p < len+10; p++)
  {
    for (int j = 0; j < len; j++)
    {
      if (((p-j) > -1) && ((p-j) < 10))
      {
        setCol(p-j, (byte)seq[j], intensity, color);
      }
    }
    delay(150);
  }
}


void putChar(char *seq, int c, int font_size)
{
  if (c == 0) 
  {
    *seq++ = 0;
  }
  else 
  {
    if ( c >= 0x60 && c <= 0x7f ) c = c - 0x20;
    if ( font_size <= FONT_3_X_5 )
    {
      for (int col=0; col < FONT_3_X_5; col++)
      {
        *seq++ = pgm_read_byte_near(&font3x5[c - 0x20][col]);
      }
    }
    else
    {
      for (int col=0; col < FONT_5_X_7; col++)
      {
        *seq++ = pgm_read_byte_near(&font5x7[c - 0x20][col]);
      }
    }
  }
  *seq++ = 0;
}


void scrollText(const char* string, unsigned int string_color, int font_size) 
{
  char scroll_buf[(font_size + 1)*50];

  for ( int i = 0; i < (font_size + 1)*50 ; i++ ) scroll_buf[i] = 0;

  for (int i = 0; i < strlen(string); i++)
  {
    putChar(scroll_buf + (i * (font_size + 1)), string[i], font_size);
  }
  scrollSequence(scroll_buf, strlen(string)*(font_size + 1), 255, string_color);
}

void outChar( char inchar, unsigned int string_color, int left_column, int font_size)
{
  char column_buffer[10];

  for ( int i = 0; i < 10 ; i++ ) column_buffer[i] = 0;

  putChar(column_buffer, inchar, font_size);

  for (int j = 0; j < font_size; j++)
  {
    setCol(font_size + left_column - j, (byte)column_buffer[j], 255, string_color);
  }
}

void setLed( int led, int brightness, unsigned int color )
{
  string_group.set_color(led, brightness, color);
}

void clearscreen() 
{
  string_group.fill_color(0,70,0,0);
}

#endif //LED_UTILS_H



