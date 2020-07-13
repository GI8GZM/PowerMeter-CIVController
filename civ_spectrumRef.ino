/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

#ifdef CIV

/* Ref (spectrum reference) functions */

/*------------------------------ sRefButton() -----------------------------------
short touch - swaps back to txpwr
long touch - saves current spectrum ref against current band
*/
void sRefButton(int tStat)
{
	int band;											// current band
	float freq, r;										// freq, spectrum reference

	if (tStat != 2)										// button short touch
	{
		eraseFrame(sRef);								// revert to txPwr
		restoreFrame(txPwr);
	}
	else
	{
		// long touch saves current radio ref to hfBand
		freq = getFreq();								// need frequency to get band
		band = getBand(freq);							// get band number
		r = getRef();									// get current spectrum ref
		hfBand[band].sRef = r;
		hfProm[band].sRef = r;							// save to EEPROM
		putBandEEPROM(currBand);							// update EEProm

		// blink frame to show write
		eraseFrame(sRef);
		delay(100);										// 0.1 sec blink to show memory write
		restoreFrame(sRef);
		displayValue(sRef, r);							// update reffarme with value
	}
}

/*------------------------------ getRef() -------------------------------
reads radio sprectrum Ref setting
Returns float (ref)
*/
float getRef()
{

	int n;												// chars read into buffer
	int u = 0, d = 0;									// units & decimals
	float ref = 0.0;											// spectrum ref
	char inBuff[12];									// civ frequency inBuff buffer

	n = civWrite(civReadRef);							// request read frequency from radio
	n = civRead(inBuff);								// buffer, printflg
	if (inBuff[3] == CIVRADIO && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		u = getBCD(inBuff[n - 4]);						// convert from BCD
		d = getBCD(inBuff[n - 3]);
	}
	ref = u + (float)d / 100.0;							// format
	if (inBuff[n - 2])
		ref = -ref;										// change sign if negative

	return ref;
}

/*------------------------- setRef() ---------------------------
get Ref and if band changed, set ref
passed: current band (int)
returns: reference (float)
*/
float setRef(int band)
{
	static int prevBand;
	float sRef;											// spectrum ref

	if (band != prevBand && band != -1)	// band change?
	{
		sRef = hfBand[band].sRef;						// get ref
		putRef(sRef);									// send spec ref to radio
		prevBand = band;
	}
	sRef = getRef();									// read ref from radio
	return sRef;
}

/*------------------------------ putRef() ---------------------------------
set radio spectrum reference
ref - spectrum reference to set
*/
void putRef(float sRef)
{
	// civWriteRef[] = 7 bytes, excluding preamble
	int u, d;											// units & decimals

	// convert to BVD format for CI-V
	if (sRef < 0)										// check if float negative
		civWriteRef[5] = 0x01;							// negative
	else
		civWriteRef[5] = 0x00;							// positive

	// convert float to BCD
	sRef = abs(sRef) * 10;								// allow for 1 decimal
	u = (int)sRef / 10;									// units
	d = (int)sRef % 10;									// decimal
	civWriteRef[3] = putBCD(u);
	civWriteRef[4] = putBCD(d);

	civWrite(civWriteRef);
}

#endif

