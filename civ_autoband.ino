/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*-------------------------------------- autoBand() -----------------------------------------------------------------------
global variable aBandCountDown is number of seconds before band change is initiated
uses Metro timer for 1 sec count
skips disabled bands. at end band goes back to start.  if all bands disabled, will stop at current frequeny
uses lab.stat for on/off signals
*/

#ifdef CIV

static int aBandCountDown = 0;								// Metro timer countdown
static float prevABandFreq = 0.0;


/*----------------------------------- aBandButton() ------------------------------------
turned on/off by touch button OR  off by change in frequency
toggles status when touched if tstat <> 0
*/
void aBandButton(int tStat)
{

	int countDown = optABand.val;					// local variable

	switch (tStat)
	{
	case 0:											// program call - initialise
		lab[aBand].stat = optABand.isFlg;			// boot time start option
		prevABandFreq = getFreq();					// initialise prevAbandFreq
		break;
	case SHORTTOUCH:								// swap on/off
		lab[aBand].stat = !lab[aBand].stat;			// toggle start/stop
		break;
	case LONGTOUCH:									// set options
		countDown = aBandCountDown;					// save current coundown
		tunerABandOpts();							// options
		drawDisplay();							
		break;
	default:										// don't come here
		break;
	}

	aBandLabel(lab[aBand].stat);					// update label
	if (lab[aBand].stat)							// if enabled
		aBandRestart(countDown);					// continue countdown
}


/*--------------------------- autoBandMain() -----------------------------------------
called by main()
*/
void autoBandMain(float freq)							// freq passed is probably current frequency
{
	if (!fr[aBand].isEnable || !lab[aBand].stat)		// check enable flag and on/off status
		return;

	// frequency manually changed? Turn off and update button
	if (freq != prevABandFreq)							// freq changed from previous
	{
		lab[aBand].stat = false;						// reset flags, stop countdown
		aBandLabel(lab[aBand].stat);					// update label
		return;
	}

	// if countdown = 0, change to next band
	if (!aBandTimer.check())							// is 1sec timer triggered (Metro timer)
		return;											// only go past here if 1 sec expires
	else
	{
		aBandCountDown--;								// display countdown 
		displayValue(aBand, aBandCountDown);
		if (aBandCountDown > 0)							// if not complete, return
			return;										// countdown not complete, return
	}
	aBandChange(freq);									// change to next valid FT8 band
	aBandRestart(optABand.val);							// restart timer
}


/*----------------------------------- aBandChange() -----------------------------
called by autoband()
*/
void aBandChange(float freq)
{
	int nextBand;

	currBand = getBand(freq);							// get current band

	nextBand = currBand + 1;							// change to next enabled frequency
	if (nextBand == NUM_BANDS)
		nextBand = 0;									// go round loop

	while (!hfBand[nextBand].isABand)					// check at least one band is enabled
	{
		nextBand++;
		if (nextBand == NUM_BANDS)
			nextBand = 0;
		if (nextBand == currBand)						// round the loop, all disabled so return
			return;										// highly unlikely
	}

	putFreq(hfBand[nextBand].ft8Freq);					// set radio to new frequency
}


/*--------------------------------- aBandLabel() -----------------------------------
displays aBand button label
called by aBandButton(), autoBand()
*/
void aBandLabel(int stat)
{
	if (!stat)											// aBand off
	{
		strcpy(lab[aBand].txt, "  ABand OFF");
		lab[aBand].colour = BUTTON_FG;
		fr[aBand].bgColour = BUTTON_BG;
		displayLabel(aBand);							// display Off label, blanks time
	}
	else												// autoband active
	{
		strcpy(lab[aBand].txt, "ABand: ");				// display label
		lab[aBand].colour = BUTTON_BG;
		fr[aBand].bgColour = BUTTON_FG;
		displayLabel(aBand);							// display Off label, blanks time
	}
}


/*------------------------aBandRestart() ----------------------------------------
called by aBandChange(), aBandButton()
used when starting or after band change
*/
void aBandRestart(int countDown)
{
	aBandCountDown = countDown;							// restart countdown from full value
	val[aBand].isUpdate = true;							// force update
	displayValue(aBand, countDown);						// display timer
	prevABandFreq = getFreq();							// save  freq
	aBandTimer.reset();									// reset 1 sec countdown timer
}

#endif