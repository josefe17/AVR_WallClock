/*
 * ADC.c
 *
 *  Created on: 4 may. 2019
 *      Author: josefe
 */

#include "ADCDriver.h"

void initADC(unsigned char prescaler, unsigned char resolution)
{
	// Select Vref=AVcc
	ADMUX = (1<<REFS0) | (1<<resolution);
	//set prescaler and enable ADC
	ADCSRA = (1<<ADEN) | (prescaler & 0x07);
}

unsigned int readADC(unsigned char ADCchannel)
{
	unsigned int readBuffer;
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while(ADCSRA & (1<<ADSC));
	if (ADMUX & (1<<ADLAR))
	{
		return (unsigned int) ADCH;
	}
	else
	{
		readBuffer = (unsigned int) ADCL;
		readBuffer |= (((unsigned int) ADCH)<<8) & 0x0300;
		return readBuffer;
	}
}

