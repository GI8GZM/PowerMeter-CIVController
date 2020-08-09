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
#define PEAK_HOLD		2000						// average Peak Pwr hold time (mSecs)
#define PEP_HOLD		250							// average pep hold time (mSecs)
#define PWR_THRESHOLD   0.5    						// power on threshold watts


/*--------------------------------------- constants for ADC -------------------------*/
//#define	AVERAGING		0							// keep = 0 for true 12 bit resolutg
//#define	RESOLUTION		12							// Teensy 4.0 max resolution
//#define	CONV_SPEED		VERY_LOW_SPEED
//#define	SAMPLE_SPEED	VERY_LOW_SPEED
#define	AVERAGING		16							// keep = 0 for true 12 bit resolutg
#define	RESOLUTION		16							// Teensy 4.0 max resolution
#define	CONV_SPEED		HIGH_SPEED
#define	SAMPLE_SPEED	HIGH_SPEED

//// these setting reduce zero offset
//#define		AVERAGING 4
//#define		RESOLUTION 16
//#define		CONV_SPEED HIGH_SPEED
//#define		SAMPLE_SPEED VERY_HIGH_SPEED



/*------  measure() constants -------------------------------*/
// offset voltages
#define	FV_ZEROADJ				-0.0001				    // ADC zero offset voltage
#define	RV_ZEROADJ				-0.0000			        // ADC zero offset voltage

// Direct power conversion constants
// forward power constants - new coupler
// 25/07/2020 - 16 bit. Load = 1k
#define	V_SPLIT_PWR         0.015

// low power = v ^ LO_EXP_PWR  * LO_MULT_PWR
#define LO_EXP_PWR			0.175
#define	LO_MULT_PWR         1				

// high power = v * v * HI_MULT2_PWR + v * MU_MULT1_PWR + HI_ADD_PWR
#define	HI_MULT2_PWR        10.408
#define HI_MULT1_PWR		3.6618
#define HI_ADD_PWR			0.5786
// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

//// 17 July 2020, 16 bit ADC, averaging = 16, speed = HIGH. load = 1K, coupler 24 turns
//// low power = v ^ LO_EXP_PWR  * LO_MULT_PWR
//#define	V_SPLIT_PWR         0.020
//#define LO_EXP_PWR			0.2181
//#define	LO_MULT_PWR         1.071				
//
//// high power = v * v * HI_MULT2_PWR + v * MU_MULT1_PWR + HI_ADD_PWR
//#define	HI_MULT2_PWR        9.5842
//#define HI_MULT1_PWR		4.1913
//#define HI_ADD_PWR			0.4588				// HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR


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
Metro pepTimer = Metro(500);				   		// pep hold timer
Metro plotTimer = Metro(50);				   		// pep hold timer

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


/* ------------   define frame positions, text formats, value display ----------------*/
// List of frame positions -------------------------
enum frameNames {
	vInVolts,			// vIn volts
	netPower,			// net Pwr
	peakPower,			// peak Pwr
	vswr,				// VSWR frame
	dBm ,				// dBm
	fwdPower,			// forward Pwr
	refPower,			// reflected Pwr
	fwdVolts,			// forward volts
	refVolts,			// reflected volts
	netPwrMeter,		// power meter
	swrMeter,			// swr meter
	avgOptions,			// options button frame
	samplesCalOpt,		// samples - calibrate
	samplesDefOpt,		// samples - default
	samplesAltOpt,		// samples - alternate
	weighting,			// weighting for exp smoothing
	freqTune,			// freqTune button frame
	aBand,				// aqutoBand button frame
	tuner,				// tuner
	band,				// band Mtrs
	sRef,				// spectrum Ref
	txPwr,				// % Tx power
	freq,				// freq Mhz
	freqTuneOpt,		// freq tune option - difference
	aBandTimeOpt,		// auto band time option
	modPlot,			// plot frame
};

// frame ------------------------------------------------------------------------
// structure definitions
struct frame {
	int x;							// top left corner - x coord
	int y;							// top left corner - y coord
	int w;							// horizontal width
	int h;							// vertical height
	int bgColour;					// frame background colour
	bool isOutLine;					// outline flg / don't display
	bool isTouch;					// frame enabled for touch
	bool isEnable;					// enable frame & CONTENTS
};

#ifdef CIV																	  
// default frames layout 
frame defFrame[] = {
//     x    y   w    h      bgColour  isOutLine isTouch isEnable
	{ 110, 75, 205, 20,		BG_COLOUR,	true,	false,	false},			// vIn volts
	{ 5, 5, 100, 90,		BG_COLOUR,	true,	true,	true},			// net Pwr
	{ 110, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// peak Pwr
	{ 215, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// VSWR frame
	{ 5, 5, 100, 85,		BG_COLOUR,	true,	false,	false},			// dBm
	{ 5, 115, 155, 30,		BG_COLOUR,	true,	false,	false},			// forward Pwr
	{ 165, 115, 155, 30,	BG_COLOUR,	true,	false,	false},			// reflected Pwr
	{ 5, 155, 155, 50,		BG_COLOUR,	true,	false,	false},			// forward volts
	{ 165, 155, 155, 50,	BG_COLOUR,	true,	false,	false},			// reflected volts
	{ 5, 95, 315, 50,		BG_COLOUR,	false,	true,	true},			// power meter
	{ 5, 95, 315, 50,		BG_COLOUR,	false,	true,	false},			// swr meter
	{ 215, 215, 100, 25,	BG_COLOUR,	true,	true,	true},			// options button frame
	{ 215, 25, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - calibrate
	{ 215, 70, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - default
	{ 215, 115, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - alternate
	{ 215, 160, 75, 40,		BG_COLOUR,	true,	false,	false},			// weighting for exp smoothing
	{ 5, 215, 100, 25,		BG_COLOUR,	true,	true,	true},			// freqTune button frame
	{ 110, 215,	100, 25,	BG_COLOUR,	true,	true,	true},			// aqutoBand button frame
	{ 5, 160, 100, 50,		BG_COLOUR,	true,	true,	true},			// tuner
	{ 110, 160, 100, 50,	BG_COLOUR,	true,	true,	true},			// band Mtrs
	{ 215, 160,	100, 50,	BG_COLOUR,	true,	false,	false},			// spectrum Ref
	{ 215, 160,	100, 50,	BG_COLOUR,	true,	true,	true},			// % Tx power
	{ 110, 160,	200, 50,	BG_COLOUR,	true,	false,	false},			// freq Mhz
	{ 200, 10,	90, 40,		BG_COLOUR,	true,	false,	false},			// freq tune option - difference
	{ 200, 125,	90, 40,		BG_COLOUR,	true,	false,	false},			// auto band time option
	{ 5, 170, 315, 35,		BG_COLOUR,	false,  false,	false},			// plot frame
};
#endif

// basic frames layout 
frame basicFrame[] = {
//     x    y   w    h      bgColour  isOutLine isTouch isEnable
	{ 110, 75, 100, 15,		BG_COLOUR,	false,	false,	false},			// vIn volts
	{ 5, 5, 100, 90,		BG_COLOUR,	true,	true,	true},			// net Pwr
	{ 110, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// peak Pwr
	{ 215, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// VSWR frame
	{ 5, 5, 100, 85,		BG_COLOUR,	true,	true,	false},			// dBm
	{ 5, 115,	155, 30,	BG_COLOUR,	true,	false,	false},			// forward Pwr
	{ 165, 115, 155, 30,	BG_COLOUR,	true,	false,	false},			// reflected Pwr
	{ 5, 155,	155, 50,	BG_COLOUR,	true,	false,	false},			// forward volts
	{ 165, 155, 155, 50,	BG_COLOUR,	true,	false,	false},			// reflected volts
	{ 5, 110,	315, 50,	BG_COLOUR,	false,	true,	true},			// power meter
	{ 5, 160, 315, 50,		BG_COLOUR,	false,	true,	true},			// swr meter
	{ 215, 215, 105, 25,	BG_COLOUR,	true,	true,	true},			// options button frame
	{ 215, 25, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - calibrate
	{ 215, 70, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - default
	{ 215, 115, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - alternate
	{ 215, 160, 75, 40,		BG_COLOUR,	true,	false,	false},			// weighting for exp smoothing
};

// calibration frames layout
frame calFrame[] = {
//     x    y   w    h      bgColour  isOutLine isTouch isEnable
	{ 110, 75, 205, 25,		BG_COLOUR,	false,	false,	true},			// vIn volts
	{ 5, 5,	100, 85,		BG_COLOUR,	true,	true,	true},			// net Pwr
	{ 110, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// peak Pwr
	{ 215, 5, 100, 65,		BG_COLOUR,	true,	true,	true},			// VSWR frame
	{ 5, 5, 100, 85,		BG_COLOUR,	true,	false,	false},			// dBm
	{ 5, 115, 155, 30,		BG_COLOUR,	true,	false,	true},			// forward Pwr
	{ 165, 115, 155, 30,	BG_COLOUR,	true,	false,	true},			// reflected Pwr
	{ 5, 155, 155, 50,		BG_COLOUR,	true,	false,	true},			// forward volts
	{ 165, 155,155, 50,		BG_COLOUR,	true,	false,	true},			// reflected volts
	{ 5, 95, 315, 55,		BG_COLOUR,	false,	false,	false},			// power meter
	{ 5, 95, 315, 55,		BG_COLOUR,	false,	false,	false},			// swr meter
	{ 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},			// options button frame
	{ 215, 25, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - calibrate
	{ 215, 70, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - default
	{ 215, 115, 75, 40,		BG_COLOUR,	true,	false,	false},			// samples - alternate
	{ 215, 160, 75, 40,		BG_COLOUR,	true,	false,	false},			// weighting for exp smoothing
};

#ifdef CIV
#define MAX_FRAMES sizeof(defFrame)/sizeof(frame)
#else
#define MAX_FRAMES 16
#endif
frame fr[MAX_FRAMES];					// working frame array - copy in defFrame, basicFrame or calFrame				

#define RADIUS 5						// frame corner radius
#define LINE_COLOUR LIGHTGREY			// frame line colour




// label -------------------------------------------------------------------
#define GAP 5						// gap from frame outline
struct label {
	char txt[30];					// frame label text
	int colour;						// text colour
	ILI9341_t3_font_t  font;		// text font size
	// xJustify -  'L'eft, 'C'entre, 'R'ight.  Value is Centred in available space
	//'F' is left justified, val spacing use val.fmt
	char xJustify;					 
	char yJustify;					// 'T'op, 'M'iddle, 'B'ottom
	int stat;						// status, used for update label display, -1 for errors
};

label lab[] = {
	{ "     Supply Volts:",	YELLOW,		FONT12,		'F', 'M', false,	},		// vIn volts
	{ "Watts",		FG_COLOUR,		FONT14,     'C', 'B', false,	},		// net Pwr
	{ "Pep",		FG_COLOUR,		FONT14,		'C', 'B', false,	},		// peak Pwr
	{ "vSWR",		FG_COLOUR,		FONT14,		'C', 'B', false,	},		// VSWR frame
	{ "dBm",		FG_COLOUR,		FONT14,		'C', 'B', false,	},		// dBm
	{ "Fwd Pwr",	YELLOW,			FONT10,		'L', 'M', false,	},		// forward Pwr
	{ "Ref Pwr",	YELLOW,			FONT10,		'R', 'M', false,	},		// reflected Pwr
	{ "Fwd Volts",	GREENYELLOW,	FONT10,		'R', 'M', false,	},		// forward volts
	{ "Ref Volts",	GREENYELLOW,	FONT10,		'L', 'M', false,	},		// reflected volts
	{ "       Watts",	FG_COLOUR,	FONT8,		'L', 'B', false,	},		// power meter
	{ "       vSWR ",	FG_COLOUR,	FONT8,		'L', 'B', false,	},		// swr meter
	{ "Avg",		BUTTON_FG,		FONT12,		'C', 'M', false,	},		// options button frame
	{ "",			CIV_COLOUR,		FONT14,		'L', 'M', false,	},		// samples - calibrate
	{ "",			CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// samples - default
	{ "",			CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// samples - alternate
	{ "",			CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// weighting for exp smoothing

#ifdef CIV
	{ "FreqTune On",BUTTON_FG,		FONT12,		'L', 'M', false,	},		// freqTune button frame
	{ "ABand ",		BUTTON_FG,		FONT12,		'L', 'M', false,	},		// aqutoBand button frame
	{ "Tune Off",	FG_COLOUR,		FONT18,		'C', 'M', false,	},		// tuner
	{ "mtrs ",		FG_COLOUR,		FONT14,		'R', 'M', false,	},		// band Mtrs
	{ "Ref ",		FG_COLOUR,		FONT14,		'R', 'M', false,	},		// spectrum Ref
	{ "%Tx ",		FG_COLOUR,		FONT14,		'R', 'M', false,	},		// % Tx power
	{ "MHz ",		CIV_COLOUR,		FONT18,		'R', 'M', false,	},		// freq Mhz
	{ " kHz",		CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// freq tune option - difference
	{ " Secs",		CIV_COLOUR,		FONT14,		'R', 'M', false,	},		// auto band time option
	{ "",			GREEN,			FONT10,		'C', 'T', true,		},		// plot frame
#endif

};

// value ---------------------------------------------------------------------------
struct value {
	float prevVal;				// previous display value
	int decs;						// decimals - not used unless sprintf not supported
	char fmt[10];					// sprintf format
	int colour;						// text colour
	ILI9341_t3_font_t  font;		// text font size
	bool isUpdate;					// true forces display update
};

value val[] = {
	{ 0.0, 1,	"%6.1f",	YELLOW,		FONT12,	    true},		// vIn volts
	{ 0.0, 0,	"%1.0f",	FG_COLOUR,	FONT40,	    true},		// net Pwr
	{ 0.0, 0,	"%1.0f",	FG_COLOUR,	FONT32,	    true},		// peak Pwr
	{ 0.0, 1,	"%3.1f",	ORANGE,		FONT28,	    true},		// VSWR frame
	{ 0.0, 0,	"%1.0f",	GREEN,		FONT40,	    true},		// dBm
	{ 0.0, 3,	"%5.2f",	ORANGE,		FONT18,	    true},		// forward Pwr
	{ 0.0, 3,	"%5.2f",	ORANGE,		FONT18,	    true},		// reflected Pwr
	{ 0.0, 5,	"%3.5f",	ORANGE,		FONT20,	    true},		// forward volts
	{ 0.0, 5,	"%3.5f",	ORANGE,		FONT20,	    true},		// reflected volts
	{ 0.0, 0,	"%3.0f",	ORANGE,		FONT18,	    true},		// power meter
	{ 0.0, 0,	"%3.0f",	ORANGE,		FONT18,	    true},		// swr meter
	{ 0.0, 0,	"%3.0f",	BG_COLOUR,	FONT16,	    true},		// options button frame
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT18,	    true},		// samples - calibrate
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT18,	    true},		// samples - default
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT18,	    true},		// samples - alternate
	{ 0.0, 3,	"%3.3f",	CIV_COLOUR,	FONT18,	    true},		// weighting for exp smoothing

#ifdef CIV
	{ 0.0, 0,	"%3.0f",	BG_COLOUR,	FONT16,	    true},		// freqTune button frame
	{ 0.0, 0,	"%3.0f",	BG_COLOUR,	FONT16,	    true},		// aqutoBand button frame
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT24,	    true},		// tuner
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT24,	    true},		// band Mtrs
	{ 0.0, 1,	"%3.1f",	CIV_COLOUR,	FONT20,	    true},		// spectrum Ref
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT24,	    true},		// % Tx power
	{ 0.0, 5,	"%3.5f",	CIV_COLOUR,	FONT24,	    true},		// freq Mhz
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT18,	    true},		// freq tune option - difference
	{ 0.0, 0,	"%3.0f",	CIV_COLOUR,	FONT18,	    true},		// auto band time option
	{ 0.0, 0,	"%3.0f",	ORANGE,		FONT18,	    true},		// plot frame
#endif
};

// meter structure
struct meter {
	float sStart;					// start value for scale
	float sEnd;						// end value for scale
	int major;						// number major scale divisions
	int tColour;					// meter bar gradient top colour
	int bColour;					// meter bar bottom colour
	int pkWidth;					// peak indicator width
	int pkColour;					// peak indicator colour
	int pkPrevPosn;					// peak indicator prev position
};

meter mtr[] = {
	{ 0, 125,	5, FG_COLOUR, BG_COLOUR,   5, YELLOW, 0, },
	{ 1.0, 3.0,	4, FG_COLOUR, BG_COLOUR,   10, ORANGE, 0, },
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


