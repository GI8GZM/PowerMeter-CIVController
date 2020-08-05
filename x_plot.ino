void plot(int p)
{
#define XMAX 320

	static int s[XMAX + 1] = {};
	int h = fr[modPlot].h;
	int y = fr[modPlot].y;
	int xStart = fr[modPlot].x;

	if (plotTimer.check())
	{
		for (int i = xStart; i < XMAX; i++)
			s[i] = s[i + 1];
		s[XMAX] = p * h / 100;

		for (int j = xStart; j < XMAX; j++)
		{
			tft.drawFastVLine(j, y - h-2, h+2, BLACK);
			tft.drawFastVLine(j, y - s[j], s[j], YELLOW);
		}

		plotTimer.reset();
	}

}


