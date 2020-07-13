#ifdef CIV
/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*
swr vs frequency plt

draw swr/freq meter scales
set power to 40 watts
set mode to RTTY

select band manually
sweep frequency across band
display swr on meter
*/

/*---------------------------------  drawMFreqScale() ----------------------------------------------
draws scale for meter.
	Inserts major and minor ticks and tick labels
	writes label for meter function
*/
void drawFreqScale(int band)
{

	//frame* fPtr = &fr[posn];
	//meter* mPtr = &mtr[posn - netPwrMeter];
	//int x = fPtr->x + mPtr->xGap;
	//int y = fPtr->y + fPtr->h - mPtr->yGap;						// base Y for meter (bottom)
	//int yTxt = y - mPtr->font.cap_height - 2;					// y coord for scale text
	//int span = fPtr->w - 2 * mPtr->xGap;						// adjust for GAP to frame
	//float scale = mPtr->sEnd - mPtr->sStart;				// get scale factor

	int major = 4;
	int minor = major * 4;
	int x = 10;
	int y = 200;
	int span = 320 - 2 * x;
	float fStart = hfBand[band].bandStart;
	float fEnd = hfBand[band].bandEnd;
	float scale = fEnd - fStart;;

	int xs;
	int yTxt = y - 10;
	int yLine = yTxt - 5;										// y coord for line (allow two rows text)


	//if (!fPtr->isEnable)
	//	return;


	tft.setTextColor(WHITE);
	tft.setFont(Arial_8);								// set font attribs

	// draw base line
	tft.drawFastHLine(x, yLine, span, WHITE);		// draw scale line

	// Minor scale ticks
	for (int i = 0; i < minor; i++)
	{		// get positions for tick
		xs = i * span / minor + x;					// x co-ord
		tft.drawFastVLine(xs, yLine, 2, WHITE);  	// minor scale ticks
	}

	// Major scale ticks
	for (int i = 0; i < major; i++)
	{		// get positions for major ticks
		xs = i * span / major + x;					// x co-ord
		tft.drawFastVLine(xs, yLine, 5, WHITE);  	// major scale tick
		if (i == 0)
			tft.setCursor(xs - 5, yTxt);						// set cursor for text
		else
			tft.setCursor(xs - 15, yTxt);						// set cursor for text

		tft.print(scale * i / (major)+fStart, 3);	// print major tick values, zero decimal
	}

	// last text label
	xs = span + x;
	tft.drawFastVLine(xs, yLine, 4, WHITE);  		// major scale last tick
	tft.setCursor(xs - 25, yTxt);							// adjust no end overlap
	tft.print(float(fEnd), 3);					// print end of scale value, 3 decimal
}



#endif







