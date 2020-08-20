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

/*---------------------------------------------------------
ADC functions and defines
ADC id triggered by interrupt interval timer
*/
ADC* adc = new ADC();							    // adc object
ADC::Sync_result result;						    // ADC result structure
IntervalTimer sampleTimer;						    // getADC interupt timer
volatile long a0Avg, a1Avg;							// circular buffer averages
volatile unsigned int a0Peak, a1Peak;				// circular buffer peak values



/* -------------------------------- get ADC() ----------------------------------------------
interrupt called by IntervalTimer - see initADC()
get raw results from ADC and enter into circular/FIFO buffer
calculates average and peak values for ACD results
------------------------------------------------------------------------------------------*/
void getADC()
{
	//digitalWriteFast(TEST_PIN, HIGH);

	volatile static int count = 0;								// ADC circular buffer sample count
	volatile static int currSamples = 0, prevSamples = 0;
	volatile static long a1Sum = 0, a0Sum = 0;					// sum of buffer samples
	volatile static unsigned int a1Sample[MAXBUF + 1] = {};		// fwd buffer used by interrupt routine
	volatile static unsigned int a0Sample[MAXBUF + 1] = {};		// ref buffer used by interrupt routine

	// samples set by options
	// set as % of buffer space
	if (samples != 0)
		currSamples = samples * MAXBUF / 100;
	else
		currSamples = 1;

	// check for change of currSamplesAvg, reset buffers, etc
	if (currSamples != prevSamples)
	{
		for (int i = 0; i < MAXBUF; i++)
		{
			a1Sample[i] = 0;
			a0Sample[i] = 0;
		}
		count = 0;
		a1Sum = 0;
		a0Sum = 0;
		a1Peak = 0;
		a0Peak = 0;
	}

	// read ADC, both channels. 16bit needs unsigned
	result = adc->analogSyncRead(FWD_ADC_PIN, REF_ADC_PIN);

	// circular / FIFO buffer (moving) averaging
	// do reflected power first to have refS[sample] for peak
	a0Sum = a0Sum - a0Sample[count];							// remove oldest from running total
	a0Sample[count] = (uint16_t)result.result_adc0;				// save result
	a0Sum = a0Sum + a0Sample[count];							// add newest to running total

	a1Sum = a1Sum - a1Sample[count];
	a1Sample[count] = (uint16_t)result.result_adc1;
	a1Sum = a1Sum + a1Sample[count];

	// peak forward sample + corresponding reflected
	if (a1Sample[count] > a1Peak)
	{
		a1Peak = a1Sample[count];
		a0Peak = a0Sample[count];
	}

	// averages
	a1Avg = a1Sum / currSamples;
	a0Avg = a0Sum / currSamples;

	// increment sample, if max currAvgSamples, back to start
	count++;
	if (count >= currSamples)
	{
		count = 0;
		// reset peaks to current sample
		a1Peak = a1Sample[count];
		a0Peak = a0Sample[count];

		//digitalWrite(TEST_PIN, !digitalRead(TEST_PIN));
	}
	prevSamples = currSamples;

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
	sampleTimer.begin(getADC, 1000000 / SAMPLE_FREQ);
}

