/*
 * NTCSensor.c
 *
 *  Created on: 4 may. 2019
 *      Author: josefe
 */

#include "NTCSensor.h"
#include <math.h>
#include "ADCDriver.h"

void NTCSensorAverageInit(unsigned char channel,  unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw)
{
	(void) AVERAGE_COUNT;
	(*averageCounter) = 0;
	(*averageAcumulator) = 0;
	(*lastValidRaw) = readADC(channel); // First raw is without average
}

double readTempKelvin(unsigned char channel, unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw)
{
	double raw;
	if (AVERAGE_COUNT > 0) // Average on
	{
		if ((*averageCounter) < (AVERAGE_COUNT)) // Number of required average samples not achieved yet
		{
			(*averageAcumulator) += readADC(channel);
			++(*averageCounter);			
		}
		if ((*averageCounter) >= (AVERAGE_COUNT)) // Number of average samples achieved (just above)
		{
			(*lastValidRaw) = (unsigned int) ((*averageAcumulator) / (unsigned long) AVERAGE_COUNT);
			(*averageAcumulator) = 0;
			(*averageCounter) = 0;			
		}
		raw = (double) (*lastValidRaw);
	}
	else
	{
		raw	= (double) readADC(channel);
	}	
	raw = (ADC_RESOLUTION-1) / raw -1;
	double resistance = SERIESRESISTOR_OHMS / raw;
	double steinhart = resistance / THERMISTORNOMINAL_OHMS;     // (R/Ro)
	steinhart = log(steinhart);                  // ln(R/Ro)
	steinhart /= BETA;                   // 1/B * ln(R/Ro)
	steinhart += 1.0 / (TEMPERATURENOMINAL); // + (1/To)
	steinhart = 1.0 / steinhart;                 // Invert
	return steinhart;
}

double readTempCelsius(unsigned char channel, unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw)
{
	return readTempKelvin(channel, AVERAGE_COUNT, averageCounter, averageAcumulator, lastValidRaw) - CESIUS2KELVIN_CONSTANT;
}

double readTempFahrenheit(unsigned char channel, unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw)
{
	return (1.8 * readTempCelsius(channel, AVERAGE_COUNT, averageCounter, averageAcumulator, lastValidRaw)) + 32.0;
}
