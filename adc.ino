/*-----------------------------------------------------------------------------------
SWR / POWER METER + IC7300 C-IV CONTROLLER

Swr/PowerMeter (basic) - https://github.com/GI8GZM/PowerSwrMeter
Swr/PowerMeter + IC7300 C-IV Controller - https://github.com/GI8GZM/PowerMeter-CIVController

© Copyright 2018-2020  Roger Mawhinney, GI8GZM.
No publication without acknowledgement to author
-------------------------------------------------------------------------------------*/


// adc.ino
// initialises analog-digital converter
// enables adc interrupt timer
// getADC() samples voltages, averages and peaks


/*--------------------------------------- constants for ADC -------------------------*/
#define	AVERAGING		0							// keep = 0 for true 12 bit resolutg
#define	RESOLUTION		12							// Teensy 4.0 max resolution
#define	CONV_SPEED		VERY_LOW_SPEED
#define	SAMPLE_SPEED	VERY_LOW_SPEED

// these setting reduce zero offset
//#define		AVERAGING 4
//#define		RESOLUTION 16
//#define		CONV_SPEED HIGH_SPEED
//#define		SAMPLE_SPEED VERY_HIGH_SPEED

/*---------------------------------------------------------
ADC functions and defines
ADC id triggered by interrupt interval timer
*/
ADC* adc = new ADC();							    // adc object
ADC::Sync_result result;						    // ADC result structure
IntervalTimer sampleTimer;						    // getADC interupt timer
volatile long fwdAvg, refAvg;						// circular buffer totals
volatile unsigned int refPk, fwdPk;					// circular buffer peak values



/* -------------------------------- get ADC() ----------------------------------------------
interrupt called by IntervalTimer - see initADC()
get raw results from ADC and enter into circular/FIFO buffer
calculates average and peak values for ACD results
peak values held for pkTimer duration
------------------------------------------------------------------------------------------*/
void getADC()
{
	//digitalWriteFast(TEST_PIN, HIGH);

	volatile static int sample = 0;							// ADC circular buffer sample
	volatile static int currAvgSamples = 0, prevAvgSamples = 0;
	volatile static long fwdSum = 0, refSum = 0;
	volatile static unsigned int fwdS[MAXBUF + 1] = {};		// fwd buffer used by interrupt routine
	volatile static unsigned int refS[MAXBUF + 1] = {};		// ref buffer used by interrupt routine

	// samples related to sample frequency
	currAvgSamples = samples * SAMPLE_FREQ / 100;
	currAvgSamples = samples * MAXBUF / 100;

	// check for change of currSamplesAvg, reset buffers, etc
	if (currAvgSamples != prevAvgSamples)
	{
		for (int i = 0; i < MAXBUF; i++)
		{
			fwdS[i] = 0;
			refS[i] = 0;
		}
		sample = 0;
		fwdSum = 0;
		refSum = 0;
		fwdPk = 0;
		refPk = 0;
	}

	// read ADC, both channels
	result = adc->analogSyncRead(FWD_ADC_PIN, REF_ADC_PIN);

	// circular / FIFO buffer (moving) averaging
	// do reflected first to have refS[sample] for peak
	refSum = refSum - refS[sample];							// remove oldest from running total
	refS[sample] = result.result_adc0;						// save result
	//refS[sample] = 0;						// save result
	refSum = refSum + refS[sample];							// add newest to running total

	fwdSum = fwdSum - fwdS[sample];
	fwdS[sample] = result.result_adc1;
	//fwdS[sample] = 0;
	fwdSum = fwdSum + fwdS[sample];

	// peak forward sample + corresponding reflected
	if (fwdS[sample] > fwdPk)
	{
		fwdPk = fwdS[sample];
		refPk = refS[sample];
	}

	// averages
	fwdAvg = fwdSum / currAvgSamples;
	refAvg = refSum / currAvgSamples;

	// increment sample, if max currAvgSamples, back to start
	sample++;
	if (sample >= currAvgSamples)
	{
		sample = 0;
		 // reset peaks to current sample
		fwdPk = fwdS[sample];
		refPk = refS[sample];

		//digitalWrite(TEST_PIN, !digitalRead(TEST_PIN));
	}
	prevAvgSamples = currAvgSamples;

	//digitalWriteFast(TEST_PIN, LOW);
}



/*---------------------------------------- initADC() ----------------------------------
initialises Analog-Digital convertor
sets resolution, conversion speeds
interrupt timer interval*/
void initADC()
{
	// set up ADC convertors - ADC 0
	adc->adc0->setAveraging(AVERAGING); 								// set number of averages,(0,4,18,16,32)
	adc->adc0->setResolution(RESOLUTION); 								// set bits of resolution (8,10,12, 16 (Teensy 3.2 )
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::CONV_SPEED);	// change the conversion speed
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::SAMPLE_SPEED);		// change the sampling speed

	// ADC 1
	adc->adc1->setAveraging(AVERAGING);
	adc->adc1->setResolution(RESOLUTION);
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::CONV_SPEED);
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::SAMPLE_SPEED);

	// set up interrupt timer (microseconds
	// SAMPLE_FREQ in hertz - eg 5000
	sampleTimer.begin(getADC, 1000000/SAMPLE_FREQ);						
}

