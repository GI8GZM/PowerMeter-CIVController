/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

// fontsColours.h
// ILI9341 fonts and colours

//#include <font_Arial.h>							// used for meter scale text
//#include <font_LiberationSansNarrowBold.h>		// main font
//#include <font_AwesomeF180.h>					// copyright symbol
//#include <font_AwesomeF000.h>					// + / - symbols


#if 0
#include <font_ArialBold.h>
#include <font_Arial.h>
#include <font_LiberationMonoBold.h>
//#include "D:\Downloads\Teensy\fonts\fonts\ofl\acme\font_AcmeRegular.h"

#include "font_LiberationMono.h"
#include "font_DroidSans.h"
#include "font_DroidSans_Bold.h"
#include "font_DroidSansMono.h"
#include "font_CourierNew.h"
#include "font_CourierNewBold.h"
#include "font_ComicSansMSBold.h"
#include "font_TimesNewRomanBold.h"
#include "font_AwesomeF100.h"
#endif

// fonts for display variables
#define FONT8 LiberationSansNarrow_8_Bold
#define FONT9 LiberationSansNarrow_9_Bold

#define FONT10 LiberationSansNarrow_10_Bold
#define FONT12 LiberationSansNarrow_12_Bold

#define FONT14 LiberationSansNarrow_14_Bold
#define FONT16 LiberationSansNarrow_16_Bold
//#define FONT18 LiberationMono_18_Bold
#define FONT18 LiberationSansNarrow_18_Bold
#define FONT20 LiberationSansNarrow_20_Bold
#define FONT24 LiberationSansNarrow_24_Bold
//#define FREQFONT LiberationMono_18_Bold
#define FREQFONT LiberationSansNarrow_24_Bold

#define FONT28 LiberationSansNarrow_28_Bold
#define FONT32 LiberationSansNarrow_32_Bold
#define FONT40 LiberationSansNarrow_40_Bold
#define FONT48 LiberationSansNarrow_48_Bold
#define FONT60 LiberationSansNarrow_60_Bold
#define FONT72 LiberationSansNarrow_72_Bold
#define FONT96 LiberationSansNarrow_96_Bold

// Color definitions
#define CL(_r,_g,_b) ((((_r)&0xF8)<<8)|(((_g)&0xFC)<<3)|((_b)>>3))	// colour(RGB)

#define BLACK       0x0000   /*   0,   0,   0 */
#define NAVY        0x000F   /*   0,   0, 128 */
#define DARKGREEN   0x03E0   /*   0, 128,   0 */
#define DARKCYAN    0x03EF   /*   0, 128, 128 */
#define MAROON      0x7800   /* 128,   0,   0 */
#define PURPLE      0x780F   /* 128,   0, 128 */
#define OLIVE       0x7BE0   /* 128, 128,   0 */
#define LIGHTGREY   0xC618   /* 192, 192, 192 */
#define DARKGREY    0x7BEF   /* 128, 128, 128 */
#define BLUE        0x001F   /*   0,   0, 255 */
#define GREEN       0x07E0   /*   0, 255,   0 */
#define CYAN        0x07FF   /*   0, 255, 255 */
#define RED         0xF800   /* 255,   0,   0 */
#define MAGENTA     0xF81F   /* 255,   0, 255 */
#define YELLOW      0xFFE0   /* 255, 255,   0 */
#define WHITE       0xFFFF   /* 255, 255, 255 */
#define ORANGE      0xFD20   /* 255, 165,   0 */
#define GREENYELLOW 0xAFE5   /* 173, 255,  47 */
#define PINK        0xF81F

// default colour
#define FG_COLOUR WHITE
//#define BG_COLOUR CL(0,32,0)    // very dark green
#define BG_COLOUR BLACK
#define CIV_COLOUR YELLOW
//#define BUTTON_FG LIGHTGREY
#define BUTTON_FG CL(160,160,160)    // medium grey
#define BUTTON_BG BLACK

#define	    FONT_PM AwesomeF000_16					// font for plus/minus symbols
#define		PLUS_SYMBOL 85							// + symbol
#define		MINUS_SYMBOL 86							// - symbol
#define     COPYRIGHT_SYMBOL 121                    // copyright symbol (AwesomeF018_14 font)
#define	    T_OFFSET 15							    // touch offset distance (pixels)
//const int TICK_SYMBOL = 12;						// Awesome_F000 character
//const int CROSS_SYMBOL = 13;
