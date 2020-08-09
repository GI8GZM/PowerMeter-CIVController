/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------
   XPT2046 touch functions
   Uses XPT2046 interrupts, check for a touch (ts.tirqTouched())
   Interrupt driven - only get here if screen touched
*/


/*------------------------------- touch() ------------------------------------------------------------
check for screen touch
returns 0 = no touch, 1 = short touch, 2 = long touch
*/
int touch()
{
	int status = 0;

	if (ts.tirqTouched())									// interrup. screen was touched
	{
		if (ts.touched())									// +ve touch
		{
			status = 1;										// status = short touch

			// check for long touch
			longTouchTimer.reset();							// Metro timer
			while (ts.touched() && status < 2)
			{
				if (longTouchTimer.check())					// long touch timer
				{
					status = 2;
					// break when long touch detected
					break;					
				}
			}
		}
	}
	return status;
}



/* -------------------- chkTouchOptions() -------------------------------------------
args: num - number offSet items to check, optionBox pointer
returns number of box touched. -1 if touched but not checkBox item
*/
int chkTouchOption(int n, bool isRepeat)
{
	int x = 0, y = 0;						// local variables
	int i;									// item touched
	int tStatus = 0;
	TS_Point p;								// touch screen result structure
	bool isTouch = false;

	do
	{
		tStatus = touch();					// check for touch
		if (!tStatus) 							// tStatus: 0 = no touch, 1 = touched, 2 = long touch
			return -1;							// false/no touch

		p = ts.getPoint();
		x = MAPX;
		y = MAPY;

		// get touch area that was touched
		for (i = 0; i <= n; i++)
		{
			// x,y between touch area width and height with offset
			if (x > tb[i].x - T_OFFSET && x < tb[i].x + T_OFFSET
				&& y > tb[i].y - T_OFFSET && y < tb[i].y + T_OFFSET)
			{
				isTouch = true;
				break;
			}
		}
	} while (!isTouch);

	if (!isRepeat)
		// empty touch buffer for excess long touch / multiple touches
		while (ts.touched());

	return i;		// return item touched
}



/*-------------------------------- chkTouchFrame() -----------------------------------------------------------
checks for touch on enabled frames
arg: num of frames
*/
int chkTouchFrame(int n)
{
	int x = 0, y = 0;							// local variables
	int i;
	TS_Point p;									// touch screen result structure
	int tStatus;								// 0= not touched, 1 = shorttouch, 2 = longtouch,
	bool isTouch = false;

	//tStatus = 1;								// default short touch. 0= not touched, 1 = shorttouch, 2 = longtouch

	do
	{
		tStatus = touch();						// check for touch
		if (!tStatus) 							// tStatus: 0 = no touch, 1 = touched, 2 = long touch
			return -1;

		p = ts.getPoint();						// get position result
		x = MAPX;
		y = MAPY;

		// get frame that was touched
		for (i = 0; i <= n; i++)				// for all frames
		{
			if (x > fr[i].x && x < (fr[i].x + fr[i].w)		// x,y between frame width and height
				&& y > fr[i].y && (y < fr[i].y + fr[i].h))
			{
				// touch enabled frame? break on first occurance for similar posn frames
				if (fr[i].isTouch)
				{
					touchActions(i, tStatus);
					isTouch = true;
					break;
				}
			}
		}
	} while (!isTouch);

	// empty touch buffer for excess long touch
	while (ts.touched());

	return i;
}

/*--------------------------------- touchActions() --------------------------------------------------------------
actions to take when frame is touched
 arg: i = frame position, tStat = 0 (program call), 1 = normal/short touch, 2 = long touch
*/
void touchActions(int button, int tStat)				// touch actions for frame i, touch status
{
	// code for each detected frame
	switch (button)
	{
	case netPower:								// frame 0
		netPwrButton(tStat);
		break;

	case peakPower:
		peakPwrButton(tStat);
		break;

	case dBm:									// switch from power to dBm
		dbmButton(tStat);
		break;

	case vswr:
		swrButton(tStat);
		break;

	case netPwrMeter:							// swap with swrmeter 
		meterButton(netPwrMeter, swrMeter);
		break;

	case swrMeter:								// swap with netPwrMeter
		meterButton(swrMeter, netPwrMeter);
		break;

	case avgOptions:							// averaging options button
		optionsButton(tStat);
		break;

#ifdef CIV
	case freqTune:								// freqTune button
		freqTuneButton(tStat);
		break;

	case aBand:									// auto band button
		aBandButton(tStat);
		break;

	case tuner:									// activate the radio tuner
		restoreFrame(tuner);
		tunerButton(tStat);
		break;

	case freq:									// display frequency (MHz) or band (mtrs)
	case band:
		freqButton(tStat);
		break;

	case txPwr:									// txPwr active, so disable it and enable ref
		txPwrButton(tStat);
		break;

	case sRef:									// radio spectrum reference level
		sRefButton(tStat);
		break;
#endif

	default:									// default - do nothin
		break;
	}
}