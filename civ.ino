/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


#ifdef CIV
/* Icom CI-V functions

*/
/*----------Icom CI-V commands------------------------------*/
char    civReadPreamble[] = { 0xFE, 0xFE,  CIVADDR, CIVRADIO };			// read from radio command preamble
char    civWritePreamble[] = { 0xFE, 0xFE,  CIVRADIO, CIVADDR };		// write command preamble
char    civReadFreq[] = { 0x03, 0xFD };								    // read frequency
char    civWriteFreq[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD };	// set frequency
char    civReadTuner[] = { 0x1C, 0x01, 0xFD };							// read tuner status
char    civWriteTuner[] = { 0x1C, 0x01, 0x02, 0xFD };					// radio tuner activate
char    civReadRef[] = { 0x27, 0x19, 0x00, 0xFD };						// read spectrum Reference
char    civWriteRef[] = { 0x27, 0x19, 0x00,0x00,0x00,0x00,0xFD };		// spectrum Reference
char	civReadTxPwr[] = { 0x14, 0x0A, 0xFD };						    // read RF Power setting
char	civWriteTxPwr[] = { 0x14, 0x0A, 0x00, 0x00, 0xFD };			    // set RF Power



/*--------------------------- putFreq() ----------------------------------------------------
write new frequency to radio 
*/
void putFreq(float freq)
{
	encodeFreq(civWriteFreq, freq);			// encode new freq
	civWrite(civWriteFreq);					// change frequency - issue CAT command
}


/*--------------------------- getFreq() ----------------------------------------------------
read CI-V frequency
issues CI-V command to read frequency from radio and tehn decoded the
frequency returned.
Returns: frequency or 0 if fail
Global: inBuff, readFreq
Calls: civRead(), decodeBCD()
int civReadPreamble[] = { 0xFE, 0xFE,  CIVADDR, CIVRADIO };			    // write command preamble
int civWriteFreq[] =    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD };	// set frequency
  */
float getFreq()
{
	int n;									// chars read into budder
	float f;
	char inBuff[12];						// civ frequency inBuff buffer

	n = civWrite(civReadFreq);				// request read frequency from radio
	if (n == 0)								// timed out
		return 0;

	n = civRead(inBuff);					// buffer, printflg
	if (inBuff[3] == CIVRADIO && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		f = (decodeFreq(inBuff) / 1000);	// decode frequency  and convert to kHz
		return f / 1000;					// return MHZ
	}
	else
		return 0;							// nothing read, return 0
}

/*--------------------------- getBand() --------------------------------------------------------------------
compares to HF band table start and end band limits
arg: float frequency (MHz).
returns: hfband band number
*/
int getBand(float freq)
{
	int cBand = -1;							// local band number
	static int prevBand;
	int isFlg = false;

	// get band   -1 = no band, 0=160mtrs, 1=80mtrs, etc.
	for (int i = 0; i < NUM_BANDS; i++)
	{
		if (freq >= hfBand[i].bandStart && freq <= hfBand[i].bandEnd)
		{
			cBand = i;
			isFlg = true;
			break;
		}
	}

	if (cBand == prevBand)						// compare to previousBand
		return cBand;						// return if no change

	// update label with band txt
	if (!isFlg)
	{
		char txt[] = "No Band ";
		displayLabel(band, txt);
		cBand = -1;
	}
	else
	{
		// do not use restoreFrame(band);
		displayLabel(band);
		val[band].isUpdate = true;			// force value update
	}

	// set global bandCurr & return
	prevBand = cBand;							// save to previousBand
	return cBand;
}





/**************************  civ functions ********************************/

/*------------------------------ civRead() -------------------------------------------------
read CIV stream from radio
reads CI-V stream from the radio until end of frame (oxFD) received
NOTE: delay required between character reads to allow for buffer fill
Returns: number of characters received or 0 if fail
Global: inBuff
Calls:
*/
int civRead(char* buff)						// read into buff, serial print if flg set
//int civRead(char* buff)					// read into buff, serial print if flg set
{
	int n = 0;
	char inChar = '\0';						// incoming character
	bool isTtimeOut = false;

	civTimeOut.reset();						// set timeout timer
	do
	{
		delay(CIV_READ_DELAY);				// delay for read buffer
		if (civSerial.available() > 0)      // character waiting?
			inChar = civSerial.read();	    // read it - empties input buffer
		if (civTimeOut.check())
		{
			isTtimeOut = true;				// timedout, set flg
			break;							// break from loop
		}
	} while (inChar != 0xFE && !isTtimeOut);	// preamble start
	buff[n++] = inChar;						// load buff with next character
	if (isTtimeOut)
		return 0;							// timeout occurred, return 0

	// get rest of frame
	civTimeOut.reset();						// set timeout timer
	do
	{
		delay(CIV_READ_DELAY);
		if (civSerial.available() > 0) 		// character waiting?
			inChar = civSerial.read();		// read it - empties input buffer
		if (civTimeOut.check())				// timeout if looping too long
		{
			isTtimeOut = true;				// set timeout flg
			break;
		}
		buff[n++] = inChar;					// load buff with next character
	} while (inChar != 0xFD && !isTtimeOut);	// check for end of inBuff (0xFD)

	if (isTtimeOut)
		return 0;							// timeout occurred, return 0
	else
		return n;							// n = number of characters read, 0 = error
}

/* ------------------------------ civWrite() ---------------------------------------------------------------
send CI-V command to radio
sends inBuff data to radio, character at a time ip to end character (0xFD)
Returns: n = chars written, 0 if error
Global: inBuff
Calls:
*/
//int civWrite(const char* buff)
int civWrite(char* buff)
{
	int n = 0;
	char inChar = '\0';						// incoming, read char
	bool isTtimeOut = false;

	// flush send buffer
	civSerial.flush();

	civTimeOut.reset();						// set timeout timer

	// send civ write preamble
	civSerial.write(civWritePreamble[0]);		// send first char
	delay(CIV_WRITE_DELAY);					// just one delay required

	for (int i = 1; i < 4; i++)				// send rest of 4 char preamble
	{
		civSerial.write(civWritePreamble[i]);
		do
		{
			if (civSerial.available() > 0) 	// echo character waiting?
				inChar = civSerial.read();
			if (civTimeOut.check())
			{
				isTtimeOut = true;
				break;
			}
		} while (inChar != civWritePreamble[i] && !isTtimeOut);	// compare to written char
	}

	if (isTtimeOut)							// timed out, return n=0 bytes
		return 0;

	// send rest of ci-v command
	civTimeOut.reset();						// set timeout timer
	do
	{										// send code + end character
		civSerial.write(buff[n]);			// write character

		do
		{
			// delay(CIV_DELAY);
			if (civSerial.available() > 0) 	// character waiting?
				inChar = civSerial.read();
		} while (inChar != buff[n]);

		if (civTimeOut.check())
		{
			isTtimeOut = true;
			break;
		}
	} while (buff[n++] != 0xFD && !isTtimeOut);	// do until end of buff

	if (isTtimeOut)
		return 0;							// return 0 if timed out
	else
		return n;							// return number of bytes
}

/*------------------------------------ decodeBCDFreq() -----------------------------------------
 function to decode BCD frequency data.
  Called By: civreadFreq()
*/
float decodeFreq(char* buff)
{
	//unsigned long m, k1, k2, h;
	long mult = 1000000;						// multiplier =  one million
	float freq = 0.0;							// decoded frequency

	for (int i = 4; i > 0; i--)					// 4 bytes, 8-4 in buffer
	{
		freq += getBCD(buff[4 + i])*mult;		// start with MHz
		mult /= 100;							// decrement each byte
	}
	return freq;								// return frequency

}

/* --------------------------------- encodeBCDFreq() -----------------------------------
function to encode float freq to CI-V format.
buff[4] - buff[8]	Called by: initAutoBand(), autoBand()
*/
void encodeFreq(char* buff, float freq)			// buffer, value   set Frequency
{
	long mult = 1000000;						// multiplier =  one million
	long  fl = freq * mult;						// convert decimal MHz freq to Hz

	for (int i = 4; i > 0; i--)					// convert 4 bytes to BCD
	{
		int x = (fl % (mult * 100)) / mult;		// convert each byte		
		buff[i] = putBCD(x);					// write to buffer
		mult = mult / 100;						// decrement mult for each byute
	}




}

/*---------------------------------- civPrintBuffer() --------------------------------
diagnostic - prints contents of civ buffer
*/
void civBuffPrint(char* buff)
{
	int n = 0;
	while (buff[n] != 0xFD)
	{
		Serial.print(buff[n], HEX);
		Serial.print(" ");
		n++;
	}
	Serial.print(buff[n++], HEX);

	Serial.print("      Chars: ");
	Serial.println(n, DEC);
}


/*----------------------------- testCiv() ---------------------------------
*/
void testCiv()
{

	char civTest[] = { 0x1C, 0x01, 0xFD };							// read tuner status

	int n;
	char inBuff[12];						// civ frequency inBuff buffer

	civWrite(civTest);				// request read frequency from radio
	n = civRead(inBuff);
	if (inBuff[3] == CIVRADIO && inBuff[n - 1] == 0xFD)	// check format of serial stream

		civBuffPrint(inBuff);
	delay(2000);

	//int s = getTunerStat();
}
#endif