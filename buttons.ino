/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

/*-----  button functions -----------------------------------------------*/

/*------------------------------ touchnetPwr() ---------------------------------
short touch (1) - switches to dBm display. any touch goes back to netPwr
long touch (2) - toggles between normal and basic mode
*/
void netPwrButton(int tStat)
{
	// short touch - display power dBm
	if (tStat == 1)
	{
		eraseFrame(netPower);
		restoreFrame(dBm);
	}

#ifdef CIV
	// long touch - swap between civ mode and basic mode
	if (tStat == 2)
	{
		isCivEnable = !isCivEnable;
		initDisplay();
	}
#endif

}


/*------------------------ dBmButton()------------------------------------
dBm displayed by netPower short touch
short touch restores to netPwr (Watts) display
*/
void dbmButton(int tStat)
{
	eraseFrame(dBm);									// erases and disables  netPwr frame
	fr[netPower].bgColour = BG_COLOUR;
	restoreFrame(netPower);
}

/*-----------------------------peakPwrButton() --------------------------------------------------
short touch(1) - swap from Peak average power to PEP
long touch(2) - calibrate mode
*/
void peakPwrButton(int tStat)
{
	// toggle between pep and peak average power
	if (tStat == 1)
		lab[peakPower].stat = !lab[peakPower].stat;
	if (lab[peakPower].stat)
		strcpy(lab[peakPower].txt, "Avg Peak");
	else
		strcpy(lab[peakPower].txt, "Pep");
	restoreFrame(peakPower);

	// toggle normal(civ) mode and calibrate mode
	static bool isCalMode = false;					// normal/calmode flag
	if (tStat == 2)
	{
		if (!isCalMode)
		{
			// calibrate
			samples = optCal.val;
			copyFrame(calFrame, sizeof(calFrame) / sizeof(frame));
			drawDisplay();
			isCalMode = true;
		}
		else
		{
			// normal mode
			samples = optDefault.val;
			isCalMode = false;

#ifdef CIV
			copyFrame(defFrame, sizeof(defFrame) / sizeof(frame));
#endif
			initDisplay();
		}
	}
}

/*---------------------- swrButton ------------------------------
short touch - swap from one decimal to two decmal places
long touch - reboot
*/
void swrButton(int tStat)
{
	// short touch - change decimals & font
	static bool isDefault = true;
	if (tStat == 1)
	{
		if (!isDefault)
		{
			val[vswr].font = FONT28;
			// copy format string
			strncpy(val[vswr].fmt, "%3.1", 4);
		}
		else
		{
			val[vswr].font = FONT24;
			strncpy(val[vswr].fmt, "%3.2", 4);
		}
		// toggle status flag
		isDefault = !isDefault;
		restoreFrame(vswr);
	}

	//reboot;
	if (tStat == 2)
	{
		// unset isEEInit flag to copy defualts to EEPROM
		//EEPROM.write(0, false);

		CPU_RESTART;
	}
}

/*--------------------meterButton() -------------------------
swap currMeter and newMeter
*/
void meterButton(int currMeter, int newMeter)
{
#ifdef CIV
	// if !civ mode, two meters enabled
	if (isCivEnable)
	{
		// normal mode
		// swap meters at same posn
		eraseFrame(currMeter);
		restoreFrame(newMeter);
		drawMeterScale(newMeter);
	}

	else
#endif
	{
		int x = fr[currMeter].x;
		int y = fr[currMeter].y;

		fr[currMeter].x = fr[newMeter].x;
		fr[currMeter].y = fr[newMeter].y;
		fr[newMeter].x = x;
		fr[newMeter].y = y;
		// swap meters
		restoreFrame(currMeter);
		drawMeterScale(currMeter);
		restoreFrame(newMeter);
		drawMeterScale(newMeter);
	}

}

#ifdef CIV
/*---------------------------- freqButton() -----------------------------------
short touch - swap frequency (Mhz) band(metres) & txPwr/ref
long touch - null
*/
void freqButton(int tStat)
{
	// short touch
	if (tStat == 1)
	{
		// if enabled, display band
		if (fr[freq].isTouch)
		{
			eraseFrame(freq);
			restoreFrame(band);
			restoreFrame(txPwr);
		}
		else
		{
			// display frequency
			eraseFrame(band);
			eraseFrame(txPwr);
			eraseFrame(sRef);
			restoreFrame(freq);
		}
	}

	if (tStat == 2)
	{
		// change up to next ft8 band
		int newBand = aBandChange(getFreq());
		float newFreq = getFreq();

		// displayValue only displays enabled value
		displayValue(band, hfBand[newBand].mtrs);
		displayValue(freq, newFreq);

		// if ABand enabled, restart new countdown
		if (lab[aBand].stat)
			aBandRestart(optABand.val);

	}
}
#endif

/*------------------------------ optionsButton ----------------------
toggles between calibrate mode and ci-v mode
tStat = 0 (don't toggle), 1 (short touch), 2 (long Touch)
Calibrate -Plot antenna rms volts (oscilloscope) against Vf.
Use curve fitting to get constants.
*/
void optionsButton(int tStat)
{
	// short touch - cycle through averaging options
	if (tStat == 1)
	{
		if (samples == optDefault.val)
			samples = optAlt.val;
		else if (samples == optAlt.val)
			samples = optCal.val;
		else if (samples == optCal.val)
			samples = optDefault.val;
		avgOptionsLabel();
	}

	//  long touch - set averaging options
	if (tStat == 2)
		setAvgSamples();
}

/*---------------------- optionsLabel() -------------------------------
update options label with averaging status
averaging (1-100: fast - slow)
*/
void avgOptionsLabel()
{
	if (samples != optDefault.val)
	{
		// invert avgOptions button
		lab[avgOptions].colour = BUTTON_BG;
		fr[avgOptions].bgColour = BUTTON_FG;
	}
	else
	{
		lab[avgOptions].colour = BUTTON_FG;
		fr[avgOptions].bgColour = BUTTON_BG;
	}

	// display avgOptions on button
	sprintf(lab[avgOptions].txt, "Average: %d", samples);
	restoreFrame(avgOptions);
}