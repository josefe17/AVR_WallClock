/*
 * NTCSensor.h
 *
 *  Created on: 4 may. 2019
 *      Author: josefe
 */

#ifndef NTCSENSOR_H_
#define NTCSENSOR_H_

#define BETA 3950.0
#define CESIUS2KELVIN_CONSTANT 273.15
#define SERIESRESISTOR_OHMS 10000.0
#define THERMISTORNOMINAL_OHMS 10000.0
#define TEMPERATURENOMINAL (CESIUS2KELVIN_CONSTANT + 25.0)

void NTCSensorAverageInit(unsigned char channel,  unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw);
double readTempCelsius(unsigned char channel,  unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw);
double readTempKelvin(unsigned char channel,  unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw);
double readTempFahrenheit(unsigned char channel,  unsigned int const AVERAGE_COUNT, unsigned int* averageCounter, unsigned long* averageAcumulator, unsigned int* lastValidRaw);

#endif /* NTCSENSOR_H_ */
