/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

// connstants and defines for Powermeter

#ifdef CIV
/*---------------------------Serial ports -------------------*/
#define	civSerial       Serial1					    // uses serial1 rx/tx pins 0,1
#define	btSerial        Serial3					    // bluetooth serial3 - pins 7,8
#endif

/*------  measure() constants -------------------------------*/
#define	SAMPLE_FREQ		5000						// effective ADC sampling frequency - hertz
#define MAXBUF			1000						// max size of circular buffers
//#define MAXBUF		SAMPLE_FREQ
#define PEAK_HOLD		2000						// average Peak Pwr hold time (mSecs)
#define PEP_HOLD		250							// average pep hold time (mSecs)
#define PWR_THRESHOLD   .5    						// power on threshold watts
#define	FV_ZEROADJ      -0.0000 				    // ADC zero offset voltage
#define	RV_ZEROADJ      -0.0000				        // ADC zero offset voltage

///*------  measure() constants -------------------------------*/
// Direct power conversion constants
// forward power constants - new coupler
// 03/06/2020
#define	FWD_V_SPLIT_PWR         0.015				// split voltage, direct pwr conversion
#define	FWD_LO_MULT_PWR         0.0969				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
#define FWD_LO_ADD_PWR          0.8719
#define	FWD_HI_MULT2_PWR        10.384
#define FWD_HI_MULT1_PWR		6.3687
#define FWD_HI_ADD_PWR			0.4894				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

//// Direct power conversion constants
//// forward power constants - new coupler
//// 01/06/2020
//#define	FWD_V_SPLIT_PWR         0.0175				// split voltage, direct pwr conversion
//#define	FWD_LO_MULT_PWR         0.068				// LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
//#define FWD_LO_ADD_PWR          0.7459
//#define	FWD_HI_MULT2_PWR        11
//#define FWD_HI_MULT1_PWR       6
//#define FWD_HI_ADD_PWR         .45				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR


#ifdef CIV
/*----------Icom CI-V Constants------------------------------*/
#define CIVADDR         0xE2			        	// this controller address
#define CIVRADIO        0x94						// Icom IC-7300 CI-V default address
#define CIV_WRITE_DELAY 3					        // CIV write delay = milliseconds
#define CIV_READ_DELAY  1						    // CIV read delay to ensure rx buffer fill
#endif


/*----------Metro timers-----------------------------------------*/

Metro netPwrTimer = Metro(50);					// net power hold timer
Metro netPwrPkTimer = Metro(1000);					// peak power hold timer
Metro pepTimer = Metro(1000);				   		// pep hold timer

#ifdef CIV
Metro civTimeOut = Metro(100);						// civ read/write watchdog timer
Metro aBandTimer = Metro(1000);						// autoband time milliseconds, auto reset
#endif


/*-------------------- expressions ------------------------------*/
// constant expression to convert BCD to decimal
constexpr int getBCD(int n)
{
	return (n / 16 * 10 + n % 16);
}

// constant expression to convert decimal to BCD
constexpr int putBCD(int n)
{
	return (n / 10 * 16 + n % 10);
}

// Frame positions --------------------------------------------------------------------------

// Default Frame position number.
const int
netPower = 0,			// net Pwr
peakPower = 1,			// peak Pwr
vswr = 2,				// VSWR frame
dBm = 3,				// dBm
fwdPower = 4,			// forward Pwr
refPower = 5,			// reflected Pwr
fwdVolts = 6,			// forward volts
refVolts = 7,			// reflected volts

netPwrMeter = 8,		// power meter
swrMeter = 9,			// swr meter

avgOptions = 10,		// options button frame
samplesCalOpt = 11,	// meaurement average - samples register size
samplesDefOpt = 12,		// meaurement average - samples register size
samplesAltOpt = 13,	// calibrate average - samples register size
weighting = 14;         // weighting for exponential smoothing+

#ifdef CIV
const int				// civ frames
freqTune = 15,			// freqTune button frame
aBand = 16,				// aqutoBand button frame

civ = 17,				// civ frame
tuner = 18,				// tuner
band = 19,				// band Mtrs
sRef = 20,				// spectrum Ref
txPwr = 21,				// % Tx power
freq = 22,				// freq Mhz

freqTuneOpt = 23,		// options - tune freq difference
aBandTimeOpt = 24;		// options - autoband time
#endif


// frame ------------------------------------------------------------------------
#define RADIUS 5				// frame corner radius
#define LINE_COLOUR LIGHTGREY	// frame line colour

#ifdef CIV
#define NUM_FRAMES 25
#else
#define NUM_FRAMES 15
#endif

// working frame array
frame fr[NUM_FRAMES];           // Copy either civFrame or Basic Frame or calFrame

// ------------------------------  basic (non civ) frame layout -------------------------------
frame basicFrame[] = {
  { 5, 10,		100, 100,	BG_COLOUR,	true,	true,	true},			// 0 - netPwr (default - netPower)
  { 110, 10,	100, 65,	BG_COLOUR,	true,	true,	true},			// 1 - peakPwr
  { 215, 10,	100, 65,	BG_COLOUR,	true,	true,	true},			// 2 - swr
  { 5, 10,		100, 100,	BG_COLOUR,	true,	true,	false},			// 3 - dBm

  { 5, 115,		155, 30,	BG_COLOUR,	true,	false,	false},			// 4 - fwdPwr (forward power)
  { 165, 115,	155, 30,	BG_COLOUR,	true,	false,	false},			// 5 - refPwr (reflected power)
  { 5, 155,		155, 50,	BG_COLOUR,	true,	false,	false},			// 6 - fwdVolts
  { 165, 155,	155, 50,	BG_COLOUR,	true,	false,	false},			// 7 - refVolts

	// meter position														 
  { 5, 110,		315, 50,	BG_COLOUR,	false,	true,	true},			// 8 - Pwr Meter
  { 5, 160,		315, 50,	BG_COLOUR,	false,	true,	true},			// 9 - SWR Meter

	// buttons
  { 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},			// 10 - avgOptions (avgOptions button)
  { 215, 25,	75, 40,		BG_COLOUR,		true,	false,	false},		// 11 - samplesDefault (averaging samples - default)
  { 215, 70,	75, 40,		BG_COLOUR,		true,	false,	false},		// 12 - samplesAltOpt	(averaging samples - alternate)
  { 215, 115,	75, 40,		BG_COLOUR,		true,	false,	false},		// 13 - samplesCalOpt	(averaging samples - calibrate mode)
  { 215, 160,	75, 40,		BG_COLOUR,		true,	false,	false},		// 14 -  weighting for expnential smoothing  

#ifdef CIV																	  
  { 5, 215,		105, 25,	BG_COLOUR,	true,	false,	false},			// 15 - freqTune (freq tune button)
  { 110, 215,	105, 25,	BG_COLOUR,	true,	false,	false},			// 16 - aBand (auto band button)

	// civ	 - do not use
  { 5, 150,		315, 65,	BG_COLOUR,	true,   false,	false},			// 17 - civ (CI-V panel, contains freq, band, txPwr, Ref)

	// tuner status
  {5, 160,		105, 45,	BG_COLOUR,	true,	false,	false},			// 18 - tuner	(radio tuner status)

  // civ data displays
  { 115, 160,	100, 45,	BG_COLOUR,	true,	false,	false},			// 19 - band (hf Band)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},			// 20 - ref (radio spectrum reference)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},			// 21 - txPwr	(radio - %TX Power)
  { 113, 160,	200, 45,	BG_COLOUR,	true,	false,	false},			// 22 - freq	(radio frequen160, 55z)

	// variables / parameters												  
  { 200, 10,	90, 40,		BG_COLOUR,		true,	false,	false},		// 23 - freqTuneOpt (options for freqTune by hf band)
  { 200, 125,	90, 40,		BG_COLOUR,		true,	false,	false},		// 24 - aBandTimeOpt (options for autoband 45ange by hf band)
#endif

};


#ifdef CIV																	  
//------------------------------------ civ (default) frame layout ------------------------------
frame civFrame[] = {
  { 5, 10,		100, 75,	BG_COLOUR,	true,	true,	true},			// 0 - netPwr (default - netPower)
  { 110, 10,	100, 65,	BG_COLOUR,	true,	true,	true},			// 1 - peakPwr
  { 215, 10,	100, 65,	BG_COLOUR,	true,	true,	true},			// 2 - swr
  { 5, 10,		100, 75,	BG_COLOUR,	true,	false,	false},			// 3 - dBm

  { 5, 115,		155, 30,	BG_COLOUR,	true,	false,	false},			// 4 - fwdPwr (forward power)
  { 165, 115,	155, 30,	BG_COLOUR,	true,	false,	false},			// 5 - refPwr (reflected power)
  { 5, 155,		155, 50,	BG_COLOUR,	true,	false,	false},			// 6 - fwdVolts
  { 165, 155,	155, 50,	BG_COLOUR,	true,	false,	false},			// 7 - refVolts

	// meter position														 
  { 5, 95,		315, 55,	BG_COLOUR,	false,	true,	true},			// 8 - Pwr Meter
  { 5, 95,		315, 55,	BG_COLOUR,	false,	true,	false},			// 9 - SWR Meter

	// buttons
  { 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},			// 10 - avgOptions (avgOptions button)
  { 215, 25,	75, 40,		BG_COLOUR,		true,	false,	false},		// 11 - samplesDefault (averaging samples - default)
  { 215, 70,	75, 40,		BG_COLOUR,		true,	false,	false},		// 12 - samplesAltOpt	(averaging samples - alternate)
  { 215, 115,	75, 40,		BG_COLOUR,		true,	false,	false},		// 13 - samplesCalOpt	(averaging samples - calibrate mode)
  { 215, 160,	75, 40,		BG_COLOUR,		true,	false,	false},		// 14 -  weighting for expnential smoothing  

  { 5, 215,		105, 25,	BG_COLOUR,	true,	true,	true},			// 15 - freqTune (freq tune button)
  { 110, 215,	105, 25,	BG_COLOUR,	true,	true,	true},			// 16 - aBand (auto band button)

	// civ	 - do not use
  { 5, 150,		315, 65,	BG_COLOUR,	false,   false,	false},			// 17 - civ (CI-V panel, contains freq, band, txPwr, Ref)

	// tuner status
  {5, 160,		105, 45,	BG_COLOUR,	true,	true,	true},			// 18 - tuner	(radio tuner status)

  // civ data displays
  { 115, 160,	100, 45,	BG_COLOUR,	true,	true,	true},			// 19 - band (hf Band)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},			// 20 - ref (radio spectrum reference)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	true,	true},			// 21 - txPwr	(radio - %TX Power)
  { 113, 160,	200, 45,	BG_COLOUR,	true,	false,	false},			// 22 - freq	(radio frequen160, 55z)

	// variables / parameters												  
  { 200, 10,	90, 40,		BG_COLOUR,		true,	false,	false},		// 23 - freqTuneOpt (options for freqTune by hf band)
  { 200, 125,	90, 40,		BG_COLOUR,		true,	false,	false},		// 24 - aBandTimeOpt (options for autoband 45ange by hf band)
};
#endif

// calibration frame layout
frame calFrame[] = {
  { 5, 10,		100, 75,	BG_COLOUR,	true,	true,	true},			// 0 - netPwr (default - netPower)
  { 110, 10,	100, 65,	BG_COLOUR,	true,	true,	true},			// 1 - peakPwr
  { 215, 10,	100, 65,	BG_COLOUR,	true,	false,	true},			// 2 - swr
  { 5, 10,		100, 75,	BG_COLOUR,	true,	false,	false},			// 3 - dBm

  { 5, 115,		155, 30,	BG_COLOUR,	true,	false,	true},			// 4 - fwdPwr (forward power)
  { 165, 115,	155, 30,	BG_COLOUR,	true,	false,	true},			// 5 - refPwr (reflected power)
  { 5, 155,		155, 50,	BG_COLOUR,	true,	false,	true},			// 6 - fwdVolts
  { 165, 155,	155, 50,	BG_COLOUR,	true,	false,	true},			// 7 - refVolts

	// meter position														 
  { 5, 95,		315, 55,	BG_COLOUR,	false,	false,	false},			// 8 - Pwr Meter
  { 5, 95,		315, 55,	BG_COLOUR,	false,	false,	false},			// 9 - SWR Meter

	// buttons
  { 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},			// 10 - avgOptions (avgOptions button)
  { 215, 25,	75, 40,		BG_COLOUR,		true,	false,	false},		// 11 - samplesDefault (averaging samples - default)
  { 215, 70,	75, 40,		BG_COLOUR,		true,	false,	false},		// 12 - samplesAltOpt	(averaging samples - alternate)
  { 215, 115,	75, 40,		BG_COLOUR,		true,	false,	false},		// 13 - samplesCalOpt	(averaging samples - calibrate mode)
  { 215, 160,	75, 40,		BG_COLOUR,		true,	false,	false},		// 14 -  weighting for expnential smoothing  

#ifdef CIV																	  
  { 5, 215,		105, 25,	BG_COLOUR,	true,	false,	false},			// 15 - freqTune (freq tune button)
  { 110, 215,	105, 25,	BG_COLOUR,	true,	false,	false},			// 16 - aBand (auto band button)

	// civ	 - do not use
  { 5, 150,		315, 65,	BG_COLOUR,	true,   false,	false},			// 17 - civ (CI-V panel, contains freq, band, txPwr, Ref)

	// tuner status
  {5, 160,		105, 45,	BG_COLOUR,	true,	false,	false},			// 18 - tuner	(radio tuner status)

  // civ data displays
  { 115, 160,	100, 45,	BG_COLOUR,	true,	false,	false},			// 19 - band (hf Band)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},			// 20 - ref (radio spectrum reference)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},			// 21 - txPwr	(radio - %TX Power)
  { 113, 160,	200, 45,	BG_COLOUR,	true,	false,	false},			// 22 - freq	(radio frequen160, 55z)

	// variables / parameters												  
  { 200, 10,	90, 40,		BG_COLOUR,		true,	false,	false},		// 23 - freqTuneOpt (options for freqTune by hf band)
  { 200, 125,	90, 40,		BG_COLOUR,		true,	false,	false},		// 24 - aBandTimeOpt (options for autoband 45ange by hf band)
#endif

};

// label --------------------------------------------------------------------------------------------------------
#define GAP 5						// gap from frame outline

//struct label {
//	char txt[30];					// frame label text
//	int colour;						// text colour
//	ILI9341_t3_font_t  font;		// text font size
//	char xJustify;					// 'L'eft, 'C'entre, 'R'ight
//	char yJustify;					// 'T'op, 'M'iddle, 'B'ottom
//	int stat;						// status, used for update label display, -1 for errors
//};

label lab[] = {
  { "Watts",		FG_COLOUR,		FONT14,     'C', 'B', false,	},		// 0 - net Pwr frame
  { "Pep",			FG_COLOUR,		FONT14,		'C', 'B', false,	},		// 1 - peak Pwr	 lab.stat = 0 for peakpwr, .stat=1 for PEP
  { "vSWR",			FG_COLOUR,		FONT14,		'C', 'B', false,	},		// 2 - VSWR
  { "dBm",			FG_COLOUR,		FONT14,		'C', 'B', false,	},		// 3 - dBm
  { "Fwd Pwr",		YELLOW,			FONT10,		'L', 'M', false,	},		// 4 - forward Pwr frame
  { "Ref Pwr",		YELLOW,			FONT10,		'R', 'M', false,	},		// 5 - reflected Pwr
  { "Fwd Volts",	GREENYELLOW,	FONT10,		'R', 'M', false,	},		// 6 - Forward voltage
  { "Ref Volts",	GREENYELLOW,	FONT10,		'L', 'M', false,	},		// 7 - Reflected Voltage
  { "         Watts",	FG_COLOUR,	FONT8,		'L', 'B', false,	},		// 8 - Pwr Meter
  { "         vSWR ",	FG_COLOUR,	FONT8,		'L', 'B', false,	},		// 9 - SWR Meter
  { "Avg",			BUTTON_FG,		FONT12,		'C', 'M', false,	},		// 10 - avgOptions button
  { "",				CIV_COLOUR,		FONT14,		'L', 'M', false,	},		// 11 - default samples size
  { "",				CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// 12 - alternate samples size
  { "",				CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// 13 - calibrate samples size
  { "",				CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// 14 - weighting

#ifdef CIV
  { "FreqTune On",	BUTTON_FG,		FONT12,		'L', 'M', false,	},		// 15 - freqtuner button
  { "ABand ",		BUTTON_FG,		FONT12,		'L', 'M', false,	},		// 16 - autoBand button
  { "",				GREEN,			FONT10,		'C', 'T', true,		},		// 17 - civ (CI-V panel, contains freq, band, txPwr, Ref)// 17-civ
  { "Tune Off",		FG_COLOUR,		FONT18,		'C', 'M', false,	},		// 18 - tuner
  { "mtrs ",		FG_COLOUR,		FONT14,		'R', 'M', false,	},		// 19 - band
  { "Ref ",			FG_COLOUR,		FONT14,		'R', 'M', false,	},		// 20 - ref
  { "%Tx ",			FG_COLOUR,		FONT14,		'R', 'M', false,	},		// 21 - % RF Power setting
  { "MHz ",			CIV_COLOUR,		FONT18,		'R', 'M', false,	},		// 22 - freq
  { " kHz",			CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// 23 - tuner freq diff
  { " Secs",		CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// 24 - aBand time, secs
#endif

};

// value ---------------------------------------------------------------------------

//struct value {
//	float prevDispVal;					// previous display value
//	int decs;							// decimals
//	int colour;							// text colour
//	ILI9341_t3_font_t  font;			// text font size
//	bool isUpdate;						// true forces display update
//};

value val[] = {
  { 0.0, 0,	 FG_COLOUR,		FONT40,	    true},		// 0 - netPwr
  { 0.0, 0,	 FG_COLOUR,		FONT32,	    true},		// 1 - peakPwr
  { 0.0, 1,	 ORANGE,		FONT28,	    true},		// 2 - swr
  { 0.0, 0,	 GREEN,			FONT48,	    true},		// 3 - dbm
  { 0.0, 2,	 ORANGE,		FONT18,	    true},		// 4 - fwdPwr
  { 0.0, 2,	 ORANGE,		FONT18,	    true},		// 5 - refPwr
  { 0.0, 4,	 ORANGE,		FONT20,	    true},		// 6 - fwdVolts
  { 0.0, 4,	 ORANGE,		FONT20,	    true},		// 7 - refVolts
  { 0.0, 0,	 ORANGE,		FONT18,	    true},		// 8 - netPwrMeter
  { 0.0, 0,	 ORANGE,		FONT18,	    true},		// 9 - swrMeter
  { 0.0, 0,	 BG_COLOUR,		FONT16,	    true},		// 10 - avgOptions button
  { 0.0, 0,	 CIV_COLOUR,	FONT18,	    true},		// 11 - measure samples size
  { 0.0, 0,	 CIV_COLOUR,	FONT18,	    true},		// 12 - measure samples size
  { 0.0, 0,	 CIV_COLOUR,	FONT18,	    true},		// 13 - calibrate samples size
  { 0.0, 3,	 CIV_COLOUR,	FONT18,	    true},		// 14 - weighting

#ifdef CIV
  { 0.0, 0,	 BG_COLOUR,		FONT16,	    true},		// 15 - freqtuner button
  { 0.0, 0,	 BG_COLOUR,		FONT16,	    true},		// 16 - autoBand button
  { 0.0, 0,	 ORANGE,		FONT18,	    true},		// 17 - civ
  { 0.0, 0,	 CIV_COLOUR,	FONT24,	    true},		// 18 - tuner
  { 0.0, 0,	 CIV_COLOUR,	FONT24,	    true},		// 19 - band
  { 0.0, 1,	 CIV_COLOUR,	FONT20,	    true},		// 20 - spectrum ref
  { 0.0, 0,	 CIV_COLOUR,	FONT24,	    true},		// 21 - %Tx Power Setting
  { 0.0, 5,	 CIV_COLOUR,	FONT24,	    true},		// 22 - freq
  { 0.0, 0,	 CIV_COLOUR,	FONT18,	    true},		// 23 - tune freq diff
  { 0.0, 0,	 CIV_COLOUR,	FONT18,	    true},		// 24 - aBand time
#endif
};

// meter -----------------------------------------------------------------------
//struct meter {
//	float sStart;						// start value for scale
//	float sEnd;							// end value for scale
//	int major;							// number major scale divisions
//	int tColour;						// meter bar gradient top colour
//	int bColour;						// meter bar bottom colour
//	int pkWidth;						// peak indicator width
//	int pkColour;						// peak indicator colour
//	int pkPrevPosn;						// peak indicator prev position
//};
//
meter mtr[] = {
{ 0, 100,	4, FG_COLOUR, BG_COLOUR,   5, YELLOW, 0, },
{ 1.0, 4.0,	4, FG_COLOUR, BG_COLOUR,   10, ORANGE, 0, },
};

#ifdef CIV
meter freqMtr = { 1.0, 4.0,	4, FG_COLOUR, BG_COLOUR,   10, ORANGE, 0, };

/*-------------------------------Frequency / Band data-----------------------------*/
#define NUM_BANDS 12
struct freqBand {
	int posn;							// hf band posn
	char txt[10];						// band description
	int mtrs;							// band - metres
	float prevFreq;						// previous frequency for update
	float ft8Freq;						// FT8 freq for autoband change
	float bandStart;					// band start freq
	float bandEnd;						// band end freq
	float sRef;							// spectrum reference
	float pwrMult;                      // pwr factor multiplier due to frequency response
	bool isFTune;						// enable tuner flag for this band
	bool isABand;						// enable autoband flag
};

freqBand hfBand[] = {
	//  posn	txt			mtrs  prevFreq   FT8Freq  BandStart   BandEnd	   Ref  pwrMult   tuneFlg  aBandFlg
		{ 0, "160 Mtrs",	160,	0.0,	 1.840,		1.81,		2.0,		0,	 1.000,   false,	false,	},
		{ 1, "80 Mtrs",		80 ,	0.0,	 3.573,		3.5,		3.8,		0,	 1.000,   false,	true,	},
		{ 2, "60 Mtrs",		60 ,	0.0,	 5.357,		5.2585,		5.4065,		0,	 1.000,   false,	false,	},
		{ 3, "40 Mtrs",		40 ,	0.0,	 7.074,		7.0,		7.2,		0,	 1.010,   true,	    true,	},
		{ 4, "30 Mtrs",		30 ,	0.0,	 10.136,	10.1,		10.15,		0,	 1.040,   false,	true,	},
		{ 5, "20 Mtrs",		20 ,	0.0,	 14.074,	14.0,		14.35,		0,	 1.050,   true,	    true,	},
		{ 6, "17 Mtrs",		17 ,	0.0,	 18.100,	18.068,		18.168,		0,	 1.050,   true,	    true,	},
		{ 7, "15 Mtrs",		15 ,	0.0,	 21.074,	21.0,		21.45,		0,	 1.015,   true,	    true,	},
		{ 8, "12 Mtrs",		12 ,	0.0,	 24.915,	24.89,		24.99,		0,	 0.97,	  true,	    true,	},
		{ 9, "10 Mtrs",		10 ,	0.0,	 28.074,	28.0,		29.7,		0,	 0.940,   true,	    true,	},
		{ 10, "6 Mtrs",		6,		0.0,	 50.313,	50.0,		52.0,		0,	 1.220,   false,    false,	},
		{ 11, "4 Mtrs",		4,		0.0,	 70.150,	70.0,		70.5,		0,	 0.868,   false,	0,	},
};
#endif

/* structure for options boxes */
struct optBox										// touch check bxes/circles co-ords
{
	int x;
	int y;
};
optBox		tb[30];									// tb[] is touch area co-ord

/*----------EEPROM Options for HF Bands----------------------------------------------------*/
// EEPROM Adresses + Increments
#define		EEINCR 16								// address increment for band options and parameters
#define		EEADDR_PARAM 10							// start address variable parameter

#ifdef CIV
#define		EEADDR_BAND 100							// start address band info

/* strucure for Options - hfBands  (12 locations 3x4) */
struct eeProm0
{
	float	sRef;									// band radio spectrum ref
	bool	isFTune;								// tuner enable flag
	bool	isABand;								// autoband enable falg
};
eeProm0		hfProm[NUM_BANDS];						 // hf bands info
#endif

/*----------eeProm strucure for Variable Parameters------------------------------*/
struct option										// param structure definition
{
	int		val;									// variables value
	bool	isFlg;									// variable flag
	int		eeAddr;									// eeProm address
};

// averaging 1-100% (fast - slow)
option		optCal = { 75, 1,	EEADDR_PARAM + 0x20 };			// number samples for averaging - default
option		optDefault = { 5,	1,	EEADDR_PARAM + 0x30 };			// alternate samples number
option		optAlt = { 25,	1,	EEADDR_PARAM + 0x40 };			// samples for averaging
option     optWeight = { 500, 1, EEADDR_PARAM + 0x50 };			// weight (x1000) for exponential averaging
#define     SAMPLES_CHANGE 5									// % change samples amount

#ifdef CIV
option		optFreqTune = { 200,	0,	EEADDR_PARAM };			// freqTune parameters
option		optABand = { 120,	0,	EEADDR_PARAM + 0x10 };		// autoband paramters
#endif


//
//
//#ifdef TEENSY40
//#define		SAMPLE_INTERVAL 10									// ADC sample timer interval (microsecs)
//#define     SAMPLES_CHANGE 500                                       // change samples amount
//param		samplesSlowPar = { 5000, 1,	EEADDR_PARAM + 0x20 };		// number samples for averaging - default
//param		samplesMedPar =	{ 2000,	1,	EEADDR_PARAM + 0x30 };		// alternate samples number
//param		samplesFastPar = { 100,	1,	EEADDR_PARAM + 0x40 };		// samples for averaging
//
//#else                                                             // TEENSY32
//#define		SAMPLE_INTERVAL 50									// ADC sample timer interval (microsecs)
//#define     SAMPLES_CHANGE 20                                     // change samples amount
//param		samplesSlowPar = { 500,	1,	EEADDR_PARAM + 0x20 };		// number samples for averaging - default
//param		samplesMedPar = { 100, 1, EEADDR_PARAM + 0x30 };		// alternate samples number
//param		samplesFastPar = { 1000,	1,	EEADDR_PARAM + 0x40 };	// samples for averaging
//
//#endif


