/*
 * ADC.h
 *
 *  Created on: 4 may. 2019
 *      Author: josefe
 */

#ifndef ADCDRIVER_H_
#define ADCDRIVER_H_

#include <avr/io.h>

#define ADC_RESOLUTION 1024

#define ADC_8BIT_RESOLUTION 1
#define ADC_10BIT_RESOLUTION 0

#define ADC_2_PRESCALER 1
#define ADC_4_PRESCALER 2
#define ADC_8_PRESCALER 3
#define ADC_16_PRESCALER 4
#define ADC_32_PRESCALER 5
#define ADC_64_PRESCALER 6
#define ADC_128_PRESCALER 7

/*Prototypes*/
/*ADC*/
void initADC(unsigned char prescaler, unsigned char resolution);
unsigned int readADC(unsigned char ADCchannel);

#endif /* ADCDRIVER_H_ */
