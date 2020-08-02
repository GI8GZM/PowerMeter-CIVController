/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

� Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/
/*
	Teensy 3.2 / 4.0
	ILI9341 TFT liquid crystal display, 240x320 dots, RGB colour.
	XPT2046 Touch screen,   Needs mapped to match TFT display

	15 May 2020 - project renamed to PowerMeter
*/


// comment following line for Basic SWR/Power Metet.  Uncomment for + C-IV control
#define		CIV										// build with CIV functions
#define		TEENSY40								// comment this line for default = Teensy 3.2
#define		TOUCH_REVERSED true 					// touchscreen, true = reversed, false = normal
#define     SCREEN_ROTATION 1						// rotation for tft and touchscreen

#include <SPI.h>
#include <ILI9341_t3.h>
#include <XPT2046_Touchscreen.h>
#include <ADC.h>
#include <EEPROM.h>
#include <Metro.h>

#include <font_Arial.h>								// used for meter scale text
#include <font_LiberationSansNarrowBold.h>			// main font
#include <font_AwesomeF180.h>						// copyright symbol
#include <font_AwesomeF000.h>						// + / - symbols

// gheader for Teensy+ ILI9341 touch display board
#include "teensyDisplay.h"

// power meter specific
#include "pwrMeter.h"								// PowerMeter defines

/* global variables  */
//int		devId = 1;								// default device to Teensy 3.2

int		samples;									// number of circular buffer samples
bool	isDim = false;								// dim flag, false = no dim

#ifdef CIV
bool	isCivEnable = true;							// 0 = Power meter only, 1 = added CI-V
int		currBand = -1;								// current band (-1 for no band)
float	currFreq = 0.0;								// current freq
#endif

/*-------------------------------------------------------------------------------------------------------
  setup() - initialise functions
  runs once
*/
void setup()
{
	//// get Teensy type (3.2 or 4.0)
	//long cpuid = CPU_ID;								// see pwrMeter.h for code definition
	//int dev_id;
	//if (cpuid == 0xC240)
		//dev_id = 1;									//T3.1	(M4)
	//else if (cpuid == 0xC270)
		//dev_id = 3;									//T4.0	(M7)

	pinMode(LED_BUILTIN, OUTPUT);						// diagnostic trigger
	pinMode(TEST_PIN, OUTPUT);							// set toggle pin for timing

	// set tft to off
	analogWrite(DIM_PIN, TFT_OFF);

	tft.begin();										// TFT begin
	tft.setRotation(SCREEN_ROTATION);					// horizontal, pins to right. Cursor 0,0 = TOP LEFT of screen

	ts.begin();											// touch screen begin
	ts.setRotation(SCREEN_ROTATION);							// set touch rotation

	// USB serial
	Serial.begin(19200);								// serial for debug
	delay(100);
	Serial.println("Swr/PowerMeter + C-IV Controller");
	Serial.println("by Gi8GZM ----------------------");


#ifdef CIV
	// civSerial
	civSerial.begin(19200);								// start teensy Serial1. RX1 - pin 0, TX1 - pin 1

	//bluetooth module HC - 05.  Default speed - 9600
	//btSerial.begin(9600);								// start Serial3. RX3 - pin 7, TX3 - pin 8,
#endif

	// initialise variables etc from EEPROM
	//clearEEPROM();									// clear EEPROM - diagnostic only
	initEEPROM();

	// set circular buffer default sample size
	samples = optDefault.val;


	// initialise ADC, set interrupt timer
	initADC();

	// display splash screen
	analogWrite(DIM_PIN, TFT_FULL);
	splashScreen();

#ifdef CIV
	// check for C-IV mode available
	// if false, disable civMode, enable basic mode
	if (!(bool)getFreq())
		isCivEnable = false;
#endif

	// draw screen etc
	initDisplay();
}

/*-----------------------------------------------------------------------------------------------
  loop() - main loop
  executes continuously
*/
void loop()
{
	//digitalWrite(TEST_PIN, !digitalRead(TEST_PIN));

	// blink indicator
	heartBeat();

	// measure & display power, swr etc - main function
	measure();

#ifdef CIV
	// blueTooth();										// test only

	// do following if civMode enabled
	if (isCivEnable)
	{
		// get and display frequency
		currFreq = getFreq();
		displayValue(freq, currFreq);

		// display band metres
		// currBand: -1(no band) or 0(160m) to 11 (4m)
		currBand = getBand(currFreq);
		if (currBand >= 0)
		{
			displayValue(band, hfBand[currBand].mtrs);
			// set spectrum ref for band
			setRef(currBand);
		}

		// tuner status / operation
		tunerMain(currFreq);

		// check for freq tune
		freqTuneMain(currFreq);

		// run FT8 auto band change
		autoBandMain(currFreq);

		// display %TX RF Power	else display spectrum ref
		if (fr[txPwr].isEnable)
			txPwrMain();
		else
			displayValue(sRef, getRef());
	}
#endif

	// dimmer timer - dim display if not active, touch to undim
	if (dimTimer.check())								// check Metro timer
		setDimmer();

	// check if screen has been touched
	if (ts.tirqTouched())
	{
		if (isDim)
			resetDimmer();								// reset dimmer
		else
			chkTouchFrame(NUM_FRAMES);					// check for touch or button press
	}
}

/*------------------------------------------------------------------------------------------
 initDisplay()
	Initialises system to standard screen layout
	called by: setup() and Options()
	used to reset the screen layout
*/
void initDisplay()
{
	// tft display OFF
	//analogWrite(DIM_PIN, TFT_OFF);
	//analogWrite(DIM_PIN, TFT_FULL);

#ifdef CIV
	// if civMode enabled
	if (isCivEnable)
	{
		// copy C-IV frame settings to working frame fr
		copyFrame(civFrame);
		val[netPower].font = FONT40;						// reset netPwr font

		// get frequency and band
		currFreq = getFreq();
		currBand = getBand(currFreq);

		// freqTuneButton label, set status = -1 to force display update
		freqTuneStatus(currFreq, -1);

		// set current frequency for freq difference tuner
		val[tuner].prevDispVal = currFreq;

		//frequency tune initialise
		// use (0) as it's a program call
		freqTuneButton(0);

		// autoband initialise
		aBandButton(0);
	}
	else
#endif
	{
		// civ disabled, copy basic swr/powermeter frame layout basics
		copyFrame(basicFrame);
		val[netPower].font = FONT48;
	}

	// draw enabled display frames labels and values
	drawDisplay();

	// empty tirqtouch buffer for first operation
	if (ts.tirqTouched())
		ts.touched();
}

/*------------------ drawDisplay -----------------------------------------------------------------------
	draw display
	displays only enabled frames
*/
void drawDisplay()
{
	// clear screen
	analogWrite(DIM_PIN, TFT_OFF);						// tft display OFF
	tft.fillScreen(BG_COLOUR);							// set tft background colour

	// draw enabled display frames, labels and values
	for (int i = 0; i < NUM_FRAMES; i++)			// do all frames, except for experimental freq meter
	{
		displayLabel(i);								// displays only enabled
		val[i].isUpdate = true;							// force values redisplay
	}

	// draw enabled meter scales
	drawMeterScale(netPwrMeter);
	drawMeterScale(swrMeter);

	// display samples / options button
	avgOptionsLabel();

	// turn screen on full bright
	analogWrite(DIM_PIN, TFT_FULL);
}

/*------------------------------------splashScreen() --------------------
displays startup messages, allows for screen calibration
*/
void splashScreen()
{
	char title[] = "";				// display current version
	char cpyRight[] = "Copyright 2020 - R Mawhinney, Gi8GZM";
	char line0[] = "SWR / Power Meter";
#ifdef CIV
	char line1[] = "+ Icom CI-V Control";
#endif
	char line2[] = "by GI8GZM";

	int y = 10;
	// display title
	tft.fillScreenVGradient(BG_COLOUR, BLUE);
	tft.setTextColor(WHITE);
	tft.setFont(FONT18);
	displayTextCentred(title, 10);

	// display copyright notice
	tft.setCursor(10, 40);
	tft.setFont(AwesomeF180_14);
	tft.write(121);
	tft.setCursor(30, 43);
	tft.setFont(FONT12);
	tft.print(cpyRight);

	// display remaining lines
	y = 100;
	tft.setFont(FONT20);
	displayTextCentred(line0, y);

#ifdef CIV
	displayTextCentred(line1, y + 40);
#endif

	tft.setFont(FONT16);
	displayTextCentred(line2, y + 100);

	// hold display
	for (int i = 0; i < SPLASH_DELAY / 10; i++)
	{
		// do other things in here
		delay(10);
	}

}

/*--------------------------- copyFrame() ---------------------------------------
copies default frame setting (frame.h) to  frame pointer
-------------------------------------------------------------------------------*/
void copyFrame(frame* fPtr)
{
	for (int i = 0; i < NUM_FRAMES; i++)
	{
		fr[i].x = fPtr[i].x;
		fr[i].y = fPtr[i].y;
		fr[i].w = fPtr[i].w;
		fr[i].h = fPtr[i].h;
		fr[i].bgColour = fPtr[i].bgColour;
		fr[i].isOutLine = fPtr[i].isOutLine;
		fr[i].isTouch = fPtr[i].isTouch;
		fr[i].isEnable = fPtr[i].isEnable;
	}
}

/*---------------------------------- resetDimmer() ---------------------
reset tft dimmer
*/
void resetDimmer()
{
	// reset dimmer flag, tft display and timer
	isDim = false;
	analogWrite(DIM_PIN, TFT_FULL);
	dimTimer.reset();
}

/*---------------------------- setDimmer() -----------------------------
tft display dimmer
*/
void setDimmer()
{
	// set flag and dim tft display
	isDim = true;
	analogWrite(DIM_PIN, TFT_DIM);
}

/*--------------------------- heartbeat() -----------------------------
heartbeat()  - timer, displays pulsing dot top left corner
*/
void heartBeat()
{
	static bool isHeartBeat;
	int x = 12, y = 12;

	if (heartBeatTimer.check())
	{
		if (isHeartBeat) {
			tft.fillCircle(x, y, 5, FG_COLOUR);
			heartBeatTimer.reset();
		}
		else
			tft.fillCircle(x, y, 5, BG_COLOUR);

		// set/reset flag, toggle indicator on/off
		isHeartBeat = !isHeartBeat;
	}
}
