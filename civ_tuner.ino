/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

#ifdef CIV

/*--------------------- tuner and frequency tune functions --------------------*/

static int	 prevTunerStatus = -1;						// previous tuner status


/* --------------------------------- tunerButton() ----------------------------------
touch activates tuner, long touch turns off Tuner at radio, disables freqTune
any touch turns on and activates tuner (radio operation).   Current tuned freq is saved.
*/
void tunerButton(int tStat)
{

	switch (tStat)
	{
	case 0:
		break;
	case SHORTTOUCH:
		tunerOnOff(true);								// activate tuner
		break;
	case LONGTOUCH:
		tunerOnOff(false);								// freq tune off
		break;
	default:
		break;
	}

	tunerMain(currFreq);								// update label, freq = 0
}
	

/*----------------------------  tunerMain() -------------------------------------------------------------------------
returns tuner status. 0=off, 1=tuner on, not activated, 2-tuning
status - 3 is "no tune" for out of band
called by: main()
*/
int tunerMain(float currFreq)							// display Tuner button status.
{
	int status = 0;

	if (!fr[tuner].isEnable)							// check enabled?
		return -1;

	if (getBand(currFreq) == -1)						// get current band
		status = 3;										// grey out tune if no band
	else
		status = getTunerStat();

	if (status == prevTunerStatus)						// if new status = previous, just return to prevent frame flicker
		return status;									// no change in status
	tunerLabel(status);									// display status

	return status;
}


/*-------------------------------- tunerLabel() -----------------------------
*/
int tunerLabel(int stat)
{
	switch (stat)
	{
		//tuner off at radio (radio startup or switched off at radio)
	case 0:
		lab[tuner].font = FONT18;						// set font and colour
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bgColour = BG_COLOUR;
		strcpy(lab[tuner].txt, "Tuner Off");
		lab[tuner].stat = false;
		break;

		// tuner on, can activated by radio or software
	case 1:
		lab[tuner].font = FONT28;
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bgColour = BG_COLOUR;
		strcpy(lab[tuner].txt, "Tune");
		lab[tuner].stat = true;							// set true 
		break;

		// tuning in operation - radio initiated
	case 2:
		lab[tuner].font = FONT24;
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bgColour = RED;								// red background
		strcpy(lab[tuner].txt, "Tuning");
		lab[tuner].stat = true;							// set true 
		break;

		// out of band, no tune
	case 3:
		lab[tuner].font = FONT18;
		lab[tuner].colour = DARKGREY;					// grey out text
		fr[tuner].bgColour = BG_COLOUR;
		strcpy(lab[tuner].txt, "No Tune");
		lab[tuner].stat = false;						// set true 
		break;

	default:											// should never get here
		break;
	}

	// update display and save status
	displayLabel(tuner);								// display Tuner label
	while (getTunerStat() == 2)							// loop and measure until done
		measure();

	prevTunerStatus = stat;								// save status
	return stat;
}


/* ------------------------------- tunerOnOff() ------------------------------------------------------
triggers the auto tuner function of the radio
arg: bool isOn - truen = activate, false = turn off
Called by: freqTune(), touch()
*/
void tunerOnOff(bool isOn)
{
	// return if disabled
	if (!fr[tuner].isEnable)
		return;

	if (isOn)
	{
		civWriteTuner[2] = 0x02;						// activate tuner 
		prevFreqTuneFreq = currFreq;							// save activated frequency
	}
	else
		civWriteTuner[2] = 0x00;						// set tuner off

	civWrite(civWriteTuner);							// set tuner on/off
}




/*----------------------------- getTunerStat() -------------------------------------------------
read tuner status from radio
issues CIV command to read Radio Tuner status
Returns: 0 = off, 1 = on, 2 = tuning
*/
int getTunerStat()
{
	int n;
	char inBuff[12];									// civ frequency inBuff buffer

	civWrite(civReadTuner);								// request read frequency from radio
	n = civRead(inBuff);
	if (inBuff[3] == CIVRADIO && inBuff[n - 1] == 0xFD)	// check format of serial stream
		return inBuff[n - 2];							// return tuner status
	else
		return -1;
}

#endif
