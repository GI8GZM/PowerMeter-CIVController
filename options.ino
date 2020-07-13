/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*----------------------------setAvgSamples()--------------------------------------------
set samples options
measure - samples register size, default & alternate
calibrate - samples register size
*/
void setAvgSamples()
{
	int x, y;
	int n;
	int samplesStat;									// samplesAvg status

	// save current samplesAvg setting, restore on exit
	if (samples == optCal.val)				
		samplesStat = 1;								
	if (samples == optDefault.val)
		samplesStat = 2;
	if (samples == optAlt.val)
		samplesStat = 3;

	tft.fillScreen(BG_COLOUR);
	tft.setTextColor(WHITE);

	// screen header, centred
	tft.setFont(FONT12);
	char txt[] = "Averaging Options: Fast - Slow (1-100%)";
	tft.setCursor((320 - tft.strPixelLen(txt)) / 2, 1);
	tft.printf(txt);

	// touch area indexing
	int tIndex = 0;
	tIndex = drawPlusMinusOpts(samplesCalOpt, "Calibrate Sampling", tIndex);
	tIndex = drawPlusMinusOpts(samplesDefOpt, "Default Sampling", tIndex);	
	tIndex = drawPlusMinusOpts(samplesAltOpt, "Alternate Sampling", tIndex);
	tIndex = drawPlusMinusOpts(weighting, "Decay Weight (0.001-1.0)", tIndex);

	x = 135; y = 210;
	drawTextBoxOpts(x, y, "Exit", tIndex);


	// touch screen options
	do
	{
		// display parameters
		displayValue(samplesCalOpt, optCal.val);
		displayValue(samplesDefOpt, optDefault.val);
		displayValue(samplesAltOpt, optAlt.val);
		displayValue(weighting, (float)optWeight.val / 1000);

		n = chkTouchOption(tIndex, true);						// check which box touched, allow repeat
		switch (n)
		{
		case 0:										// increment sample size, limit to max
			if (optCal.val == 1)
				optCal.val += SAMPLES_CHANGE - 1;
			else
				optCal.val += SAMPLES_CHANGE;
			if (optCal.val >= 100)
				optCal.val = 100;
			break;
		case 1:										// decrement sample size, min = 1
			optCal.val -= SAMPLES_CHANGE;
			if (optCal.val <= 1)
				optCal.val = 1;
			break;
		case 2:										// increment sample size, limit to max
			if (optDefault.val == 1)
				optDefault.val += SAMPLES_CHANGE - 1;
			else optDefault.val += SAMPLES_CHANGE;
			if (optDefault.val >= 100)
				optDefault.val = 100;
			break;
		case 3:										// decrement sample size, min = 1
			optDefault.val -= SAMPLES_CHANGE;
			if (optDefault.val <= 1)
				optDefault.val = 1;
			break;
		case 4:										// increment calibrate sample size
			if (optAlt.val == 1)
				optAlt.val += SAMPLES_CHANGE - 1;
			else optAlt.val += SAMPLES_CHANGE;
			if (optAlt.val >= 100)
				optAlt.val = 100;
			break;
		case 5:										// decrement calibrate sample size, min = 1
			optAlt.val -= SAMPLES_CHANGE;
			if (optAlt.val <= 1)
				optAlt.val = 1;
			break;
		case 6:
			// exponential weighting * 1000, ie 50 = 50/1000 = .05
			if (optWeight.val > 50)
				optWeight.val += 20;
			else
				optWeight.val += 1;
			// max: 1000/1000 = 1.0
			if (optWeight.val >= 1000)
				optWeight.val = 1000;
			break;
		case 7:
			// decrement calibrate sample size, min = 1
			if (optWeight.val > 50)				// 0.1
				optWeight.val -= 20;
			else
				optWeight.val -= 1;
			// min: 1/1000 = .001
			if (optWeight.val <= 1)				
				optWeight.val = 1;
			break;
		default:
			break;
		}
		// do while touched item is less than total items
	} while (n < tIndex);								


	switch (samplesStat)							// reset samplesAvg using new values
	{
	case 1:
		samples = optCal.val;
		break;
	case 2:
		samples = optDefault.val;
		break;
	case 3:
		samples = optAlt.val;
	}

	// save to EEPROM
	EEPROM.put(optCal.eeAddr, optCal);
	EEPROM.put(optDefault.eeAddr, optDefault);
	EEPROM.put(optAlt.eeAddr, optAlt);
	EEPROM.put(optWeight.eeAddr, optWeight);

	// clean up
	initDisplay();
}

/*-------------------------drawPlusMinusOpts()--------------------------------------
	draw text and value, +/- buttons
	returns touch index = original i+2
*/
int drawPlusMinusOpts(int posn, const char* txt, int tIndex)
{
	int x = 20, y;
	int offSet = 10;
	frame* fPtr = &fr[posn];

	ILI9341_t3_font_t fnt = FONT14;
	tft.setFont(fnt);
	tft.setTextColor(WHITE);
	y = fr[posn].y + fnt.cap_height;
	tft.setCursor(x, y);
	tft.printf(txt);
	restoreFrame(posn);

	// font for Plus - minus symbols
	tft.setFont(FONT_PM);
	tft.setTextColor(WHITE);

	// draw +, save + touch index, x,y
	x = fPtr->x + fPtr->w + 5;
	y = fPtr->y;
	tft.setCursor(x, y);
	tft.write(PLUS_SYMBOL);
	x += offSet;
	y += offSet;
	tb[tIndex].x = x;
	tb[tIndex].y = y;
	tIndex++;

	// draw -, save touch index, x,y
	x = fPtr->x + fPtr->w + 5;
	y = y + 10;
	tft.setCursor(x, y);
	tft.write(MINUS_SYMBOL);
	x += offSet;
	y += offSet;
	tb[tIndex].x = x;
	tb[tIndex].y = y;
	tIndex++;

	// return new touch index
	return tIndex;
}



/*------------------------drawTextBoxOpts()----------------------------
draws touch box with txt with touch index

args: x,y, text in box, drawTouchBoxOpts[num]
returns:	box width
*/
int drawTextBoxOpts(int x, int y, const char* txt, int tIndex)
{
	int r = 5;
	int s, w, h;						//  string length, box width, height adjustment
	ILI9341_t3_font_t fnt = FONT16;
	char t[10];

	strcpy(t, txt);
	// set font, get width and height from font & text
	tft.setFont(fnt);
	s = tft.strPixelLen(t);							// width of string
	w = s + 20;										// box width
	h = fnt.cap_height + 10;
	tft.drawRoundRect(x, y, w, h, r, WHITE);
	tft.setCursor(x + (w - s) / 2, y + 5);
	tft.print(txt);

	tb[tIndex].x = x + w / 2;
	tb[tIndex].y = y + 10;

	// return width
	return w;
}

/*------------------drawCircleOpts()---------------------------------------
darw filled / blank circle at x, y position
flag: 1 = filled circle
tiindex: touch box index
*/
void drawCircleOpts(int x, int y, bool isFlg, int tIndex)
{
	int r = 10;

	if (isFlg)
	{
		// normal fill
		tft.fillCircle(x, y, r, WHITE);
		tft.drawCircle(x, y, r, WHITE);
	}
	else
	{
		// alternate B/Ground fill
		tft.fillCircle(x, y, r, BG_COLOUR);
		tft.drawCircle(x, y, r, WHITE);
	}
	// save touch index, x&y
	tb[tIndex].x = x;
	tb[tIndex].y = y;
}


