/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

/*---------------------------------  eePromInit() ---------------------------------------------------------
Initialises EEPROM to default values from frame.h
   Normal start up reads EEPROM into band data and global variables
   EEPROM(0) is initialised flag, 0=not, 1= done
------------------------------------------------------------------------------------------*/
void initEEPROM(void)
{
	// new EEPROM may be initialised to 0,1, 0xFFFF, or something else
	// check isEEInit to a pattern unlilkely to be in iniial EEPROM content

	int pattern = B0101001;										// random pattern
	int isEEInit = EEPROM.read(0);								// if true = already initialised at first boot, false = new processor, uninitialised EEPROM

	if (isEEInit == pattern)
	{
		// EEProm has been initialised.  Get values and set variables
#ifdef CIV
		for (int i = 0; i < NUM_BANDS; i++)
		{
			int eeAddr = EEADDR_BAND + EEINCR * i;
			EEPROM.get(eeAddr, hfProm[i]);

			hfBand[i].sRef = hfProm[i].sRef;
			hfBand[i].isFTune = hfProm[i].isFTune;
			hfBand[i].isABand = hfProm[i].isABand;
		}
		// get variables / parameters
		EEPROM.get(optFreqTune.eeAddr, optFreqTune);
		EEPROM.get(optABand.eeAddr, optABand);
#endif

		EEPROM.get(optCal.eeAddr, optCal);
		EEPROM.get(optDefault.eeAddr, optDefault);
		EEPROM.get(optAlt.eeAddr, optAlt);
		EEPROM.get(optWeight.eeAddr, optWeight);
	}
	else
	{
		// initialise EEPROM from default values.  Should only happen with blank eeprom
		clearEEPROM();
#ifdef CIV
		for (int i = 0; i < NUM_BANDS; i++)
		{
			hfProm[i].sRef = hfBand[i].sRef;
			hfProm[i].isFTune = hfBand[i].isFTune;
			hfProm[i].isABand = hfBand[i].isABand;
			putBandEEPROM(i);
		}
		// init variables
		EEPROM.put(optFreqTune.eeAddr, optFreqTune);
		EEPROM.put(optABand.eeAddr, optABand);
#endif

		EEPROM.put(optCal.eeAddr, optCal);
		EEPROM.put(optDefault.eeAddr, optDefault);
		EEPROM.put(optAlt.eeAddr, optAlt);
		EEPROM.put(optWeight.eeAddr, optWeight);

		// set EEPROMinitialised flag
		EEPROM.write(0, pattern);								// write isEEInit flag pattern, address 0
	}
}

void clearEEPROM()
{
	// clear eeprom to zeros
	for (int i = 0; i < EEPROM.length(); i++)
		EEPROM.write(i, 0);
}


#ifdef CIV
// EEPROM put functions for hfBand data.  
/*-------------------------- putBandEEPROM() -------------------
puts band date to EEPROM
---------------------------------------------------------------*/
void putBandEEPROM(int bNum)
{
	int eeAddr = EEADDR_BAND + EEINCR * bNum;
	EEPROM.put(eeAddr, hfProm[bNum]);
}
#endif