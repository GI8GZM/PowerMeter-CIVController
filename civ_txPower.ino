/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*  Radio %Pwr control functions
civ txpwr is 0-255. Represents 0-100%
Only map before display to prevent accumulated rounding errors
*/

#ifdef CIV

/*--------------------------- txPwrButton() ----------------------------------------------------------------
short touch - toggles set TXPower between original setting and 100%
long touch  - changes to spectrum reference
*/
void txPwrButton(int tStat)
{
	int pwr = 0;
	static int prevPwr;									// prev setting
	static bool isPwrSet = false;						// pwr set flag

	if (tStat == 2)										// long touch
	{
		eraseFrame(txPwr);								// erase power
		restoreFrame(sRef);								// display ref
		displayValue(sRef, getRef());					// update ref frame with value
	}
	else												// short touch, change power setting
	{
		if (!isPwrSet)									// set %Tx Power  = 100%
		{
			prevPwr = getTxPwr();						// save current power setting
			pwr = 255;									// full 100% power
			isPwrSet = true;							// set flag
		}
		else
		{												// reset %Tx Power to previous level
			pwr = prevPwr;
			isPwrSet = false;							// reset flag
		}

		putTxPwr(pwr);									// set power 
		txPwrMain();									// display %txPower
	}
}

/*------------------------------------- displayTXPwr()----------------------------------
map before display, avoid rounding errors
change colour for highest powers
*/
void txPwrMain()
{
	int pwr = getTxPwr();

	pwr = map(pwr, 0, 255, 0, 100);						// check for 100% power
	if (pwr >= 90)					// greater than 90% and 100% set
		val[txPwr].colour = RED;						// change colour
	else
		val[txPwr].colour = CIV_COLOUR;					// set normal colour

	displayValue(txPwr, pwr);
}

/*-------------------------------- getTxPwr() --------------------------------------------------------
reads RF Power setting from radio
Returns pwr = 0-255 (0-100%)
int	civReadTxPwr[] =    { 0x14, 0x0A, 0xFD };			// read RF Power setting
*/
int getTxPwr()
{
	int n;												// chars read into budder
	unsigned int h = 0, u = 0;							// hundreds, units
	char inBuff[12];										// civ frequency inBuff buffer
	int pwr = 0;

	civWrite(civReadTxPwr);								// request read power setting from radio
	n = civRead(inBuff);								// get number of characters in buffer (9)
	if (inBuff[3] == CIVRADIO && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		h = getBCD(inBuff[n - 3]);						// hundreds, convert from BCD
		u = getBCD(inBuff[n - 2]);						// units
	}
	pwr = h * 100 + u;									// add hundreds and units to get power
	return pwr;
}

/*------------------------------ putTxPwr() -------------------------------
set Tx %power, 0-100.
range 0-255 (= 0-100%), converts decimcal to BCD, write C-IV command
returns pwr

int	civWriteTxPwr[] =   { 0x14, 0x0A, 0x00, 0x00, 0xFD };	// set RF Power

*/
void putTxPwr(int pwr)
{
	unsigned int h, u;									// hundreds and units

	h = pwr / 100;										// 100s
	u = pwr % 100;										// units

	civWriteTxPwr[2] = putBCD(h);						// constant expression
	civWriteTxPwr[3] = putBCD(u);						

	civWrite(civWriteTxPwr);							// write it, 0-255
}

#endif
