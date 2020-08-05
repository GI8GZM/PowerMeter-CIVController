/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

� Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

//#ifdef TEENSY40
//#define SAMPLE_INTERVAL 200						    // ADC sample timer interval (microsecs)
//#define MAXBUF          10000						// maximum averaging buffer/samples size
//
//#else
//#define SAMPLE_INTERVAL 400						    // ADC sample timer interval (microsecs)
//#define MAXBUF          2000						// maximum averaging buffer/samples size
//
//#endif
//


/*---------- Teensy pin assignments (use for wiring) --------------------------------------
board: referes to ILI9341 + Touch board   */
#define	DIM_PIN         3							// board: 8(LED) analog out pin for display LED & dimming
#define	TFT_DC_PIN      9                           // board: 5(DC) tft data pin
#define	TFT_CS_PIN      10							// board: 10(CS) TFT chip select pin
#define	TS_IRQ_PIN      2							// board: 14(T_IRQ) touch interrupt pin
#define	TS_CS_PIN       6							// board: 6(T_CS) Touch CS. *** do NOT use pin 8...  required for Serial3
#define TFT_SDI         11                          // board: 6(SDI), 12(T_DIN)     *** used in library, not referenced in code
#define TFT_SCK         13                          // board: 7(SCK), 10(T_CLK)     *** used in library, not referenced in code
#define TFT_SDO         12                          // board: 9(SDO), 13(T_DO)      *** used in library, not referenced in code

#define FWD_ADC_PIN     15                          // (A1) ACD input pin - forward power
#define REF_ADC_PIN     16                          // (A2) ACD input pin- reflected power
#define VIN_ADC_PIN		18							// ADC VCC measurement
#define SERIAL_RX1      0                           // Serial1 RX pin
#define SERIAL_TX1      1                           // Serial1 Tx pin
#define SERIAL_RX3      7                           // Serial3 RX pin
#define SERIAL_TX3      8                           // serial3 Tx pin
#define	TEST_PIN        4							// high/low pulse output for timing

/*----------ILI9341 TFT display (320x240)-------------------------*/
ILI9341_t3	tft = ILI9341_t3(TFT_CS_PIN, TFT_DC_PIN);		// define tft device
//ILI9341_t3(uint8_t _CS = 10, uint8_t _DC=9, uint8_t _RST = 255, uint8_t _MOSI = 11, uint8_t _SCLK = 13, uint8_t _MISO = 12);

#define		TFT_FULL 255							// tft display full brightness
#define		TFT_DIM	20								// tft display dim value
#define		TFT_OFF	0								// tft display off
#define		SPLASH_DELAY 5 * 1000					// splash screen delay, mSecs.. At power on, allow time for radio to boot

/*----------XPT2046 touchscreen	-----------------------------*/
XPT2046_Touchscreen ts(TS_CS_PIN, TS_IRQ_PIN);		// allows touch screen interrupts
#if			TOUCH_REVERSED
int xMapL = 3850, xMapR = 500;                      // reversed touch screen mapping
int yMapT = 3800, yMapB = 300;
#else
int xMapL = 320, xMapR = 3850;                      // touch screen mapping
int yMapT = 300, yMapB = 3800;
#endif
#define		MAPX map(p.x, xMapL, xMapR, 0, 320)		// touch screen mapping expresssion
#define		MAPY map(p.y, yMapT, yMapB, 0, 240)

const int SHORTTOUCH = 1;
const int LONGTOUCH = 2;


/* ---------------- Metro Timers -----------*/
Metro heartBeatTimer = Metro(500);			        // heartbeat timer
Metro longTouchTimer = Metro(750);			        // long touch timer
Metro dimTimer = Metro(15 * 60 * 1000);				// dimmer timer (mins)


 // set up default arguements for functions
void displayValue(int posn, float curr, bool isUpdate = false);
void displayLabel(int posn, char* = NULL);


/*---------- Teensy restart code (long press on Peak Power frame)--------*/
#define		CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define		CPU_RESTART_VAL 0x5FA0004
#define		CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

/*----------- code to get Teeny type -----------------------------------*/
//uint32_t cpuid = ((*(volatile uint32_t*)0xE000ED00) & 0xfff0);
//
//uint16_t dev_id;
//if (cpuid == 0xC240) dev_id = 1;		//T3.1	(M4)
//else if (cpuid == 0xC600) dev_id = 2;	//TLC	(M0+)
//else if (cpuid == 0xC270) dev_id = 3;	//T4.0	(M7)
//else dev_id = 0;

#define     CPU_ID ((*(volatile uint32_t*)0xE000ED00) & 0xfff0);




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
#define		FONT_HB AwesomeF000_10					// font for heartbeat
#define		HEART_SYMBOL 4							// heart symbol
#define     COPYRIGHT_SYMBOL 121                    // copyright symbol (AwesomeF018_14 font)
#define	    T_OFFSET 15							    // touch offset distance (pixels)
//const int TICK_SYMBOL = 12;						// Awesome_F000 character
//const int CROSS_SYMBOL = 13;
