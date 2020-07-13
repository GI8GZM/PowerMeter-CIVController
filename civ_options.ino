/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

#ifdef CIV

/*------------------- tunerABandOpts() --------------------------------------------
Options 0 - enter here after long touch on FreqTune or ABand buttons

darw tunerAband option
set the options
check exit option touched

calls freqTimeOpts() to draw check circles/ boxes for tuning and auto band
calls setParamOpts to set tuner frequency difference and band change time
*/
void tunerABandOpts()							// set frequency difference to trigger autotune
{
	int tNum, chkNum;							// drawTouchBoxOpts Number, number selected box

	// loop until Exit touched
	do
	{
		tft.fillScreen(BG_COLOUR);
		tft.setTextColor(FG_COLOUR);

		// reset chkNum for loop to work
		chkNum = 0;

		// draw check boxes for startBand to endBand + Exit & More..
		tNum = drawTunerABandOpts();

		// accept option changes until Exit or More.. is touched
		do
		{
			if (ts.tirqTouched())
			{
				chkNum = chkTouchOption(tNum, false);
				int bNum = chkNum / 2;				// band number

				// get number of item touched. Ignore -1 (touched but no item)
				if (chkNum < NUM_BANDS * 2 && chkNum != -1)
				{
					// even number selected - freqTune Options
					if (!(chkNum % 2))
					{
						hfBand[bNum].isFTune = !hfBand[bNum].isFTune;
						drawCircleOpts(tb[chkNum].x, tb[chkNum].y, hfBand[bNum].isFTune, chkNum);
					}
					// odd number - freqTune Options
					else
					{
						hfBand[bNum].isABand = !hfBand[bNum].isABand;
						drawCircleOpts(tb[chkNum].x, tb[chkNum].y, hfBand[bNum].isABand, chkNum);
					}
					// update EEPROM
					hfProm[bNum].isFTune = hfBand[bNum].isFTune;
					hfProm[bNum].isABand = hfBand[bNum].isABand;
					putBandEEPROM(bNum);					// save data to EEPROM
				}
			}
		} while (chkNum < NUM_BANDS * 2);

		// bNum is either Exit or More..(last touch item - tNum)
		if (chkNum == tNum)
			setTunerAbandOpts();						// More... selected
	} while (chkNum != (tNum - 1));				// Exit
}

/*--------------------- drawTunerABandOpts() --------------------
Options screen 1
drawTuneBandOpts() - draw check circles for AutoBand options
returns: number off check boxes including Exit and More...
*/
int drawTunerABandOpts()
{
	int x = 20, y = 70;
	const int xd = 50, yd = 25;
	int numBoxes = 0;								// drawTouchBoxOpts index
	int w;
	int xStart = 20, yStart = 60;
	int colWidth = 170;
	//TS_Point p;							// touch screen result structure

	// screen header, centred
	tft.setFont(FONT14);
	char txt[] = "Freq Tuning and AutoBand Enable";
	displayTextCentred(txt, 2);

	// column labels
	tft.setFont(FONT12);
	tft.setCursor(2, 30);
	tft.printf("Tune  ABand   Band");
	tft.setCursor(2 + colWidth, 30);
	tft.printf("Tune  ABand   Band");

	// check boxes (circles actually!)  two per band
	x = xStart;
	y = yStart;

	for (numBoxes = 0; numBoxes < NUM_BANDS; numBoxes++)
	{
		// new set?
		if (numBoxes >= 6)
		{
			x = xStart + colWidth;
			y = yStart + yd * (numBoxes - 6);
		}
		else
			y = yStart + yd * numBoxes;

		// tuneFlg are even, two circles per line
		drawCircleOpts(x, y, hfBand[numBoxes].isFTune, numBoxes * 2);

		// space to next column
		x += xd;
		drawCircleOpts(x, y, hfBand[numBoxes].isABand, numBoxes * 2 + 1);

		// display band
		tft.setCursor(x + 10, y - 5);
		tft.printf("%8d%s", hfBand[numBoxes].mtrs, " m");

		x -= xd;			// reset x
	}

	// allow for 2 items per line, starting at 0
	numBoxes = numBoxes * 2;
	// draw exit boxes
	x = 170; y = 210;
	w = drawTextBoxOpts(x, y, "Exit", numBoxes++);
	x += w;
	w = drawTextBoxOpts(x, y, "More...", numBoxes);

	// return max drawTouchBoxOpts index
	return numBoxes;
}

/*----------------------- setFreqTimerOpts() -------------------------
set tune freq difference and ft8 band change time
*/
void setTunerAbandOpts()
{
	int x = 40, y;
	int tIndex = 0, n = 0;
	bool isFlg;
	char txt[] = "On at Startup";

	tft.fillScreen(BG_COLOUR);
	tft.setFont(FONT14);
	tft.setTextColor(WHITE);

	// freqtune status
	y = fr[freqTuneOpt].y + 50;
	drawCircleOpts(x, y, optFreqTune.isFlg, tIndex);
	tft.setCursor(x + 20, y - 5);
	tft.printf(txt);
	tIndex++;

	// autoband startup status
	y = fr[aBandTimeOpt].y + 50;
	drawCircleOpts(x, y, optABand.isFlg, tIndex);
	tft.setCursor(x + 20, y - 5);
	tft.printf(txt);
	tIndex++;

	// Draw Tuner Freq Difference section
	drawPlusMinusOpts(freqTuneOpt, "Tuner Freq Diff:", tIndex);
	tIndex += 2;
	displayValue(freqTuneOpt, optFreqTune.val);

	// Draw AutoBand section
	drawPlusMinusOpts(aBandTimeOpt, "AutoBand Timer:", tIndex);
	tIndex += 2;
	displayValue(aBandTimeOpt, optABand.val);

	// draw exit box
	x = 135; y = 210;
	drawTextBoxOpts(x, y, "Exit", tIndex);

	// check for touch changes
	do
	{
		// check which box touched
		n = chkTouchOption(tIndex, false);
		isFlg = false;

		switch (n)
		{
		case 0:
			// set/reset Freq Tune Flag for startup
			optFreqTune.isFlg = !optFreqTune.isFlg;
			isFlg = optFreqTune.isFlg;
			drawCircleOpts(tb[n].x, tb[n].y, isFlg, n);
			break;
		case 1:
			// set/reset aBand Flag for startup
			optABand.isFlg = !optABand.isFlg;
			isFlg = optABand.isFlg;
			drawCircleOpts(tb[n].x, tb[n].y, isFlg, n);
			break;
		case 2:
			// increase / decrease frquency
			optFreqTune.val += 20;
			break;
		case 3:
			optFreqTune.val -= 20;
			if (optFreqTune.val <= 0)
				optFreqTune.val = 20;
			break;
			// increase/decrease time
		case 4:
			optABand.val += 15;
			break;
		case 5:
			optABand.val -= 15;
			if (optABand.val <= 0)
				optABand.val = 15;
			break;

			// Exit box
		case 6:
		default:
			break;
		}

		// update display
		displayValue(freqTuneOpt, optFreqTune.val);
		displayValue(aBandTimeOpt, optABand.val);

		// save structures to EEPROM
		EEPROM.put(optFreqTune.eeAddr, optFreqTune);;
		EEPROM.put(optABand.eeAddr, optABand);;
	} while (n != tIndex);		// last item is Exit

	// clean up
	eraseFrame(freqTuneOpt);
	eraseFrame(aBandTimeOpt);
}

#endif
