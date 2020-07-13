/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

#ifdef CIV

static float prevFreqTuneFreq = 0.0;					// previous freq at tuner activate
static int	 prevFreqTuneStatus = -1;					// previous freqTune Status

// function declaration
int freqTuneStatus(float freq, int status = prevFreqTuneStatus);


/*------------------------------- freqTuneButton() --------------------------------
  freqTuneButton()   -  called by touch()
  lab[freqTune].stat - 0: freqTune OFF,  1: freqTune ON
*/
void freqTuneButton(int tStat)
{
	switch (tStat)
	{
	case 0:
		lab[freqTune].stat = optFreqTune.isFlg;			// start up value, set by options, saved in EEPOROM
		if (lab[freqTune].stat)
			fr[freqTune].bgColour = BUTTON_FG;
		prevFreqTuneFreq = getFreq();					// set prev freq to avoid immediate tune at boot
		break;
	case SHORTTOUCH:									// swap on/off
		lab[freqTune].stat = !lab[freqTune].stat;
		break;
	case LONGTOUCH:										// set options
		tunerABandOpts();
		drawDisplay();								// restore display + freq difference
		break;
	default:											// don't come here
		break;
	}

	// for all conditions
	if (lab[freqTune].stat)								// if on, enable tuner
		lab[tuner].stat = true;							// if ftune enabled, enable Tuner
	freqTuneStatus(currFreq);							// display freqTuner status
	//displayValue(freqTune, 999, true);



}


/*------------------------- freqTuneMain() ------------------------------------------
checks for change in freq beyound freqTunePar.val and activates the radio tuner if enabled.
called by main()
*/
void freqTuneMain(float freq)
{
	if (!fr[freqTune].isEnable)							// check enable flag and on/off status
		return;

	int stat = freqTuneStatus(freq);					// get status
	if (stat != 1)										// return unless ftune on
		return;

	float freqDiff = abs(freq - prevFreqTuneFreq) * 1000;			// freq diff - kHz
	float value = constrain(optFreqTune.val - freqDiff, 0, 9999);
	displayValue(freqTune, value,true);

	if (freqDiff >= optFreqTune.val)					// freq change > limit
	{
		tunerOnOff(true);								// activate tuner. prevfreqtuneFreq is updated
	}
}


/*------------------------------ freqTuneStatus() ---------------------------
args: frequency, prevStatus   (-1 to force display update)
get status depending on band, option and tuner status
called by freqTune
*/

int freqTuneStatus(float freq, int status)
{
	int stat = 0;										// initialise ftStat
	int band = getBand(freq);

	if (!lab[freqTune].stat)
		stat = 0;										// freqtune off
	else
	{
		if (lab[tuner].stat)							// Tuner enabled
			stat = 1;

		if (!hfBand[band].isFTune)						// option disabled 
			stat = 2;

		if (band == -1)									// out of band
			stat = 3;
	}

	// check for return to prevent display flicker
	if (stat == status)									// same as previous, don't update to prevent flicker
		return stat;

	// display label status text
	lab[freqTune].colour = BUTTON_FG;					// set button fg & bg colours
	fr[freqTune].bgColour = BUTTON_BG;
	switch (stat)
	{
	case 0:
		strcpy(lab[freqTune].txt, " F/Tune OFF");		// freqtune off
		displayLabel(freqTune);							// display label only
		break;
	case 1:
		strcpy(lab[freqTune].txt, "F/Tune: ");			// freqTune normal
		invertLabel(freqTune);							// white b/g
		val[freqTune].isUpdate = true;					// force display of kHz
		displayValue(freqTune, optFreqTune.val);
		break;
	case 2:
		strcpy(lab[freqTune].txt, " FT Disabled");		// freqTune ON, disabled for band
		displayLabel(freqTune);							// display label only
		break;
	case 3:
		strcpy(lab[freqTune].txt, " Out of Band");		// frequency out of band
		displayLabel(freqTune);							// display label only
		break;
	default:
		break;
	}

	prevFreqTuneStatus = stat;								// save status and return it
	return stat;
}

#endif