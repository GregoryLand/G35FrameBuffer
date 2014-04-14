// font.h contains the column pattern for a 3 x 5 matrix defining the columns

#ifndef FONT_H
#define FONT_H

#define FONT_3_X_5  3
#define FONT_4_X_6  6
#define FONT_5_X_7  5

// Workaround for Greg's Compiler
#define prog_uchar const uint8_t

extern prog_uchar /* PROGMEM */ font3x5[64][FONT_3_X_5] ;
extern prog_uchar /* PROGMEM */ font5x7[64][FONT_5_X_7] ;

#endif

