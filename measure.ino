/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/

/*--------------------------- measure() ------------------------------------------
   reads ADC values recorded by ADC using interrupt timer - getADC().
   calculates forward, reflected power, net power, peak envelope power
   and peak power
   swr calculated from fwd and ref peak power
*/
void measure()
{
	unsigned long fA =0, rA=0, fPk=0, rPk=0;										// variables from adc interrupts
	float  fwdV=0, refV=0, fwdPkV, refPkV;									// calulated ADC voltages
	float fwdPwr = 0.0, refPwr=0.0, fwdPkPwr, refPkPwr;							// calculated powers
	float netPwr, pep, dB;
	static float pkPwr = 0, swr = 1.0;

	static float fwdVPrev = 0, refVPrev = 0;							// variables for exponential smoothing
	static float fwdPkVPrev = 0, refPkVPrev = 0;						// variables for exponential smoothing

	float vIn;
	float adcConvert = 3.3 / adc->adc0->getMaxValue();					// 3.3 (max volts) / adc max value, varies with resolution+

	// set true for netPower display (red b/ground)
	lab[netPower].stat = true;

	// main measuring / display loop
	// continue while power > threshold
	do
	{
		digitalWrite(TEST_PIN, !digitalRead(TEST_PIN));					// toggle test pin to HALF frequency

		// measure radio input volts, 4k7 / 1k  divider
		int va = (uint16_t)adc->analogRead(VIN_ADC_PIN);
		vIn = (float)va * adcConvert * 5.7;

		// get ACD results - stop / restart interrupts while copying interrupt data
		noInterrupts();
		fA = fwdAvg;
		rA = refAvg;
		fPk = fwdPk;
		rPk = refPk;
		interrupts();

		// calculate voltages
		fwdV = fA * adcConvert + FV_ZEROADJ;
		refV = rA * adcConvert + RV_ZEROADJ;
		fwdPkV = fPk * adcConvert + FV_ZEROADJ;
		refPkV = rPk * adcConvert + RV_ZEROADJ;

		// apply exponential smoothing
		float weight = (float)optWeight.val / 1000;
		weight = 1;
		fwdV = sigProcess(fwdV, fwdVPrev, weight);
		fwdVPrev = fwdV;
		refV = sigProcess(refV, refVPrev, weight);
		refVPrev = refV;
		weight = 1;
		fwdPkV = sigProcess(fwdPkV, fwdPkVPrev, weight);
		fwdPkVPrev = fwdPkV;
		refPkV = sigProcess(refPkV, refPkVPrev, weight);
		refPkVPrev = refPkV;

		//calculate power(watts) directly from voltages
		fwdPwr = pwrCalc(fwdV);
		refPwr = pwrCalc(refV);
		fwdPkPwr = pwrCalc(fwdPkV);
		refPkPwr = pwrCalc(refPkV);

		// net power (watts)
		netPwr = fwdPwr - refPwr;
		if (netPwr < 0)
			netPwr = 0.0;

		// peak power (watts), default pep
		//short touch to select pep or netPwrPeak
		if (lab[peakPower].stat)
		{
			// average net power peak
			if (pkPwr < netPwr)
			{
				pkPwr = netPwr;
				netPwrPkTimer.reset();
			}
			else if (netPwrPkTimer.check())
				pkPwr = netPwr;
		}
		else
		{
			// pep - peak envelope power, cannot be less than netpwr
			pep = fwdPkPwr - refPkPwr;
			if (pep < netPwr)
				pep = netPwr;
			// get hold
			if (pkPwr < pep)
			{
				pkPwr = pep;
				pepTimer.reset();
			}
			else if (pepTimer.check())
				pkPwr = pep;
		}


		// dBm - select by netPower longtouch
		// =  10* log10 (1000 * watts) cannot be less than 0
		dB = 10 * log10(netPwr * 1000);
		if (dB < 0)
			dB = 0.0;


		// swr calculation - only calculate if power on. do not use netPwr as signal processed
		// use power, not volts, as curve linearity already compensated
		// peak power preferred - stops SWR changes on power off as netpower decreases
		if (fwdPkPwr > PWR_THRESHOLD && fwdPkPwr > refPkPwr)
		{
			// reflection coefficient
			float rc = sqrt(refPkPwr / fwdPkPwr);
			if (rc == NAN || isnan(rc))
				swr = 1.0;
			else
			{
				swr = (1 + rc) / (1 - rc);
				if (swr <= 1.0)
					swr = 1.0;
				if (swr > 999.9)
					swr = 999.9;
			}

			// swr display colour based on value
			int swrColour = GREEN;
			if (swr > 1.5)
			{
				// change through yellow to orange to red for swr > 1.5
				int grn = map(swr, 1.5, 3.0, 255, 0);
				grn = constrain(grn, 0, 255);
				swrColour = CL(255, grn, 0);
			}
			val[vswr].colour = swrColour;
		}

		// display net power, if power on, use RED background
		// reduce display flicker, set lab[netPower].stat = false
		if (netPwr > PWR_THRESHOLD)
		{
			if (fr[netPower].isEnable && lab[netPower].stat)
			{
				fr[netPower].bgColour = RED;
				restoreFrame(netPower);
				// ensure doesn't change to RED next time
				lab[netPower].stat = false;
			}
		}

		displayValue(vInVolts, vIn);
		displayValue(netPower, netPwr);
		displayValue(dBm, dB);
		displayValue(peakPower, pkPwr);
		displayValue(vswr, swr);
		displayValue(fwdPower, fwdPwr);
		displayValue(refPower, refPwr);
		displayValue(fwdVolts, fwdV);
		displayValue(refVolts, refV);
		drawMeter(netPwrMeter, netPwr, pkPwr);
		drawMeter(swrMeter, swr, 1);
		//plot((int(pep)));

#ifdef CIV
		// get and display frequency, needed here for swr / frequency manual sweep
		// this takes 30 mSecs, so only update if SWR meter and freq display is enabled
		// useful for SWR checking vs Frequency
		if (fr[swrMeter].isEnable && fr[freq].isEnable)
			displayValue(freq, getFreq());
#endif

		// check for dimmed screen and reset
		if (netPwr >= PWR_THRESHOLD)
			resetDimmer();

		// check if screen has been touched
		if (ts.tirqTouched())
			chkTouchFrame(sizeof(fr) / sizeof(frame));

		// ensure measure loop slower than getADC() sample frequency
		// measure loop = 30microsecs, delay(>5) millisecs
		//delay(5);

		// do at least once and while power applied
	} while (netPwr >= PWR_THRESHOLD);

	// power off - display exit power
	if (fr[netPower].isEnable && !lab[netPower].stat)
	{
		fr[netPower].bgColour = BG_COLOUR;
		restoreFrame(netPower);
		displayValue(netPower, 0, true);
	}
}



/*----------------------------------- pwrCalc() ----------------------------------------------------------------
calculates pwr in Watts directly from ADC volts
applies constants from calibration procedure
if CIV enabled, adjust for freq response of coupler
*/
float pwrCalc(float v)
{
	float pwr = 0.0;

	// low power below split (non-linear)
	if (v < V_SPLIT_PWR)
		pwr = (pow(v, LO_EXP_PWR)) * LO_MULT_PWR;
	else
		// high power
		pwr = v * v * HI_MULT2_PWR + v * HI_MULT1_PWR + HI_ADD_PWR;

	// check for division by zero and < 0
	if (pwr < 0 || !isnormal(pwr) || isnan(pwr))
		pwr = 0.0;

#ifdef CIV	
	// adjust power for freq response of coupler
	//if (isCivEnable)
	//	return (pwr * hfBand[currBand].pwrMult);			// return power (watts)
	//else
#endif

	return pwr;
}


/*----------------- sigProcess --------------------------------------
signal processing - fast attack, exponential decay
weight controls decay
*/

float sigProcess(float currSig, float prevSig, float weight)
{
	float sig;

	//if (currSig > prevSig)
	//	// fast attack - immediate step up to higher value
	//	sig = currSig;
	//else
		 // exponential decay
	sig = weight * currSig + (1 - weight) * prevSig;

	return sig;
}
