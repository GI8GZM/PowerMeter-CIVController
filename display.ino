/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


/*
display functions
drawframe(), displayLabel(), displayValue(), drawMeterScale(), displayMeter()
invertLabel(), eraseFrame();
*/


/*---------------------------------------  displayLabel() + Str ----------------------------------------
displayLabel(int posn)  or displayLabel(int post, char* text)
displays frame and text at label posn
Calls:	drawFrame()

*/

void displayLabel(int posn, char* txt)						// call with frame postion + str
{
	int x, y;
	frame* fPtr = &fr[posn];
	label* lPtr = &lab[posn];

	if (txt == NULL)										// if called with 1st arg only
		txt = lab[posn].txt;								// set text

	if (!fPtr->isEnable) return;							// check enabled

	// draw associated frame
	drawFrame(posn);

	// set label txt colour and font size
	tft.setTextColor(lPtr->colour);
	tft.setFont(lPtr->font);

	// horizontal justify label position in frame
	switch (lPtr->xJustify)
	{
	case 'R':												// right justified
		x = fPtr->x + fPtr->w - tft.strPixelLen(txt) - GAP;
		break;
	case 'C':												// centered
		x = fPtr->x + (fPtr->w - tft.strPixelLen(txt)) / 2;
		break;
	case 'L':												// default -left justified
	default:
		x = fPtr->x + GAP;
	}

	// vertical justify
	switch (lPtr->yJustify)
	{
	case 'T':												// top of frame
		y = fPtr->y + GAP;
		break;
	case 'M':												// middle of frame
		y = fPtr->y + (fPtr->h - lPtr->font.cap_height) / 2;
		break;
	case 'B':												// bottom, default
	default:
		y = fPtr->y + fPtr->h - GAP - lPtr->font.cap_height;
		break;
	}

	// print label text in frame
	tft.setCursor(x, y);									// x, y = top left of text
	tft.print(txt);
}

/*------------------------------  displayValue() --------------------------------------------------
displayValue(int posn, float currVal) or displayValue(int posn, float currVal, bool isUpdate)
updates value if changed from previous of isUpdate = true
*/
void displayValue(int posn, float currVal, bool isUpdate)	// frame position, float current value to display
{
	frame* fPtr = &fr[posn];
	value* vPtr = &val[posn];
	label* lPtr = &lab[posn];
	int xCurr = 0, xPrev = 0, y = 0;
	const int buffSize = 20;									// char buffer size
	char strCurr[buffSize + 1] = {},							// char buffers for converted string
		strPrev[buffSize + 1] = {},
		strKeep[buffSize + 1] = {};
	int pixLenCurr, pixLenPrev, pixLenLabel, pixLenKeep;		// pixel lentghs of string values

	// return if disabled
	if (!fPtr->isEnable) return;

	// convert floats to strings and compare to detect position changes
	// scan strings left to right.  erase from changed position
	// use pixel length of strKeep to calculate blanking rectangle  ie - remaining digits to right

	// convert float values to strings
	sprintf(strCurr, vPtr->fmt, currVal);
	sprintf(strPrev, vPtr->fmt, vPtr->prevVal);

	// NOTE: sprintf may not work with Arduino - use following
	//int digits = 0;												// do not set number of digits
	//dtostrf(vPtr->prevDispVal, digits, vPtr->decs, strPrev);
	//dtostrf(currVal, digits, vPtr->decs, strCurr);

	// compare current and prev, return if no change
	if (!vPtr->isUpdate && !isUpdate)
		if (!strcmp(strPrev, strCurr))
			return;

	// initialise keep buffer - this the left->right string to keep.
	for (int i = 0; i < buffSize; i++)
		strKeep[i] = '\0';

	// keep string strKeep - this not erased
	// set to prev if current different to previous
	for (int i = 0; i < buffSize; i++)
	{
		if (strPrev[i] == strCurr[i])
			strKeep[i] = strPrev[i];
	}


	// work out x,y position of erase rectangle and start of value string

	// set font attribs
	tft.setFont(vPtr->font);
	tft.setTextColor(vPtr->colour);

	// pixel length of value strings
	pixLenCurr = tft.strPixelLen(strCurr);
	pixLenPrev = tft.strPixelLen(strPrev);
	pixLenKeep = tft.strPixelLen(strKeep);

	// different values with same start digit cause problems with erase, eg 10.23 and 100.34
	// compare first characters and string lengths
	if (strPrev[0] == strCurr[0] && pixLenPrev != pixLenCurr)
		pixLenKeep = 0;

	// check for label y position, adjust value to be middle of free space
	// depends if label vertical text justify - (T)op, (M)iddle or (B)ottom of frame
	switch (lPtr->yJustify)
	{
	case 'M':
		y = fPtr->y + (fPtr->h - vPtr->font.cap_height) / 2;
		break;
	case 'T':
		y = fPtr->y + (fPtr->h - vPtr->font.cap_height) / 2;
		break;
	case 'B':
		y = fPtr->y + (fPtr->h - vPtr->font.cap_height - lPtr->font.cap_height) / 2;
		break;
	}

	// get pixel postions, depending on xJustify and label x position.
	// display value in half remaining space

	// xCurr is current value position, xPrev is previous value
	// ensure label font is used.  save/restore value font
	tft.setFont(lPtr->font);
	pixLenLabel = tft.strPixelLen(lPtr->txt);
	switch (lPtr->xJustify)
	{
	case 'C':
		xCurr = fPtr->x + (fPtr->w - pixLenCurr) / 2;
		xPrev = fPtr->x + (fPtr->w - pixLenPrev) / 2;
		break;
	case 'L':
		xCurr = fPtr->x + (fPtr->w + pixLenLabel - pixLenCurr) / 2;
		xPrev = fPtr->x + (fPtr->w + pixLenLabel - pixLenPrev) / 2;
		break;
	case 'R':
		xCurr = fPtr->x + (fPtr->w - pixLenLabel - pixLenCurr) / 2;
		xPrev = fPtr->x + (fPtr->w - pixLenLabel - pixLenPrev) / 2;
		break;
	case 'F':
		// label is Left justified, value spacing uses val.fmt
		xCurr = fPtr->x + pixLenLabel + 5;
		xPrev = fPtr->x + pixLenLabel + 5;
		break;
	}
	tft.setFont(vPtr->font);


	// with positions worked out - erase to right from first changed digit
	// draw rectangle with background colour to erase
	tft.fillRect(xPrev - 2 + pixLenKeep, y - 2, pixLenPrev - pixLenKeep + 2, vPtr->font.cap_height + 5, fPtr->bgColour);

	// update current value and overprint entire string
	// this will overprint keep section (no flicker) and draw erased portion
	tft.setCursor(xCurr, y);
	tft.print(strCurr);

	// save to previous value
	vPtr->prevVal = currVal;
	// reset update flag
	vPtr->isUpdate = false;
}


/*---------------------------------  drawMeter() --------------------------------------------
Draws the meter bar in the frame
*/
void drawMeter(int posn, float curr, float peak)
{
	frame* fPtr = &fr[posn];											// display value in this frame
	value* vPtr = &val[posn];
	meter* mPtr = &mtr[posn - netPwrMeter];							// adjust for array position

	// if frame disabled return
	if (!fPtr->isEnable) return;

	// y axis parameters
	int y = fPtr->y + 10;												// vertical posn for meter
	//int yLine = y + fPtr->h - 10;										// baseline Y for meter scale (same as drawMeterScale)
	int barHeight = fPtr->h - 30;										// meter bar height

	// meter span and widths
	// match scale start position, adjust for GAP to frame
	int x = fPtr->x + 2;
	int span = fPtr->w - 10;

	// pixel positions based on span, start from x origin
	int xCurr = x - mPtr->pkWidth + map(curr, mPtr->sStart, mPtr->sEnd, 0, span);
	if (xCurr < x)
		xCurr = x;
	int xPrev = x - mPtr->pkWidth + map(vPtr->prevVal, mPtr->sStart, mPtr->sEnd, 0, span);
	if (xPrev < x)
		xPrev = x;
	int xPk = x - mPtr->pkWidth + map(peak, mPtr->sStart, mPtr->sEnd, 0, span);


	// erase previous peak indicator if peak posn has changed
	int xPkPrev = mPtr->pkPrevPosn;
	if (xPkPrev != xPk)
		tft.fillRect(xPkPrev, y, mPtr->pkWidth + 2, barHeight, mPtr->bColour);

	// erase or draw portion of meter bar that changed.
	// draw background colour to erase. adjust widths(+1) to remove "tails")
	if (xCurr < xPrev)
		tft.fillRect(xCurr, y, xPrev - xCurr + 1, barHeight, fPtr->bgColour);
	else if (xCurr > xPrev)
		tft.fillRectVGradient(xPrev, y, xCurr - xPrev + 1, barHeight, mPtr->tColour, mPtr->bColour);
	// save previous display value
	vPtr->prevVal = curr;


	// draw new peak if position has changed
	// for low power < pkWidth, at/near zero, make width variable depending on xPk 
	if (xPk != xPkPrev && xPk >= x)
		tft.fillRectVGradient(xPk, y, mPtr->pkWidth, barHeight, mPtr->pkColour, mPtr->bColour);
	else if (xPk != xPkPrev && xPk <= x)
		// display from x origin with varaible width for low power
		tft.fillRectVGradient(x, y, mPtr->pkWidth - x + xPk, barHeight, mPtr->pkColour, mPtr->bColour);

	// save peak position to previous
	mPtr->pkPrevPosn = xPk;
}



/*---------------------------------  drawMeterScale() ----------------------------------------------
draws scale for meter.
	Inserts major and minor ticks and tick labels
	writes label for meter function
*/
void drawMeterScale(int posn)
{
	frame* fPtr = &fr[posn];
	meter* mPtr = &mtr[posn - netPwrMeter];

	// return if not enabled
	if (!fPtr->isEnable)
		return;

	// x,y start posns. allow for gaps to frame
	int x = fPtr->x + 2;
	int y = fPtr->y + 10;

	// base line and text vertival posns
	int yLine = y + fPtr->h - 30;
	int yTxt = yLine + 10;

	int span = fPtr->w - 10;
	float scale = mPtr->sEnd - mPtr->sStart;

	int major = mPtr->major;
	int minor = 5 * major;
	int colour = WHITE;

	// decimals for scale values
	int decs = 0;
	if (scale < 10)
		decs = 1;

	tft.setTextColor(colour);
	tft.setFont(Arial_8);									// set font attribs

	// draw base line
	tft.drawFastHLine(x, yLine, span, colour);				// draw scale line

	int xtick;												// x posn for tick
	// Minor scale ticks
	for (int i = 0; i < minor; i++)
	{
		xtick = i * span / minor + x;						// tick x co-ord
		tft.drawFastVLine(xtick, yLine, 2, colour);  		// minor scale ticks, 2 pixels
	}

	// Major scale ticks
	for (int i = 0; i <= major; i++)
	{
		xtick = i * span / major + x;						// tick x co-ord
		tft.drawFastVLine(xtick, yLine, 5, colour);  		// major scale, 5 pixels

		// tick text. Special cases for first and last
		if (i == 0)
			tft.setCursor(xtick, yTxt);						// first text posn, inset right set cursor for text
		else if (i == major)
			tft.setCursor(xtick - 12, yTxt);				// last text posn, inset left - adjust no end overlap
		else
			tft.setCursor(xtick - 5, yTxt);					// normal tick text posn

		tft.print(scale * i / major + mPtr->sStart, decs);	// print major tick values, decimals
	}
}


/*------------------------------- drawFrame() ----------------------------------------------
frame position
draws frame if enabled, fills background colour and draws outline
*/
void drawFrame(int posn)
{
	frame* fPtr = &fr[posn];

	// draw frame if enabled
	if (fPtr->isEnable)
	{
		// filled rectangle
		tft.fillRoundRect(fPtr->x, fPtr->y, fPtr->w, fPtr->h, RADIUS, fPtr->bgColour);
		// draw outline
		if (fPtr->isOutLine)
			tft.drawRoundRect(fPtr->x, fPtr->y, fPtr->w, fPtr->h, RADIUS, LINE_COLOUR);
	}
}

/*------------------------------- eraseFrame() ----------------------------------------------------
   erases frame, outline and text to background
*/
void eraseFrame(int posn)
{
	fr[posn].isEnable = false;							// disable flags
	fr[posn].isTouch = false;
	// fill inside frame with background
	tft.fillRoundRect(fr[posn].x, fr[posn].y,			// erase frame - fill with background colour
		fr[posn].w, fr[posn].h, RADIUS, BG_COLOUR);
}

/*--------------------------------  restoreFrame() -------------------------------------------------
arg: frame position
restores frame after possible mess up such as font change, or overwite by other data
*/
void restoreFrame(int posn)
{
	eraseFrame(posn);									// erases and disables frame
	fr[posn].isEnable = true;							// enable flags
	fr[posn].isTouch = true;
	drawFrame(posn);									// redraw frame
	displayLabel(posn);									// redisplay label
	val[posn].isUpdate = true;							// force value redraw
}


/*---------------------------------- displayTextCentred() ---------------------------------------
displays lines of horizontally centered text , at vertical y
*/
void displayTextCentred(char* line, int y)
{
	int x = (320 - tft.strPixelLen(line)) / 2;
	tft.setCursor(x, y);
	tft.print(line);
}


/*------------------------------- invertLabel() ----------------------------------------
inverts current text colour and background colour
*/
void invertLabel(int posn)
{
	int col = fr[posn].bgColour;										// save bg colour
	fr[posn].bgColour = lab[posn].colour;							// swap colour
	lab[posn].colour = col;
	displayLabel(posn);										// draw label
}
