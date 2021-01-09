/*
 * fastSkipTimer.c
 *
 *  Created on: 17 dic. 2020
 *      Author: josefe
 */

#include "fastSkipTimer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

unsigned int fast_skip_count;
volatile unsigned int fast_skip_counter;
volatile unsigned char fast_skip_enable;


void start_fast_skip_timer(unsigned int timeout)
{
	fast_skip_count=timeout;
	fast_skip_counter=0;
	fast_skip_enable=1;
}

void stop_fast_skip_timer(void)
{
	fast_skip_enable=0;
}

unsigned char is_fast_skip_timer_over(void)
{
	unsigned char buffer;
	if (!fast_skip_enable)
	{
		return 0;
	}
	cli(); // Ooops 16 bit => ATOMIC
	buffer = fast_skip_counter >= fast_skip_count;
	sei();
	return buffer;
}

void increment_fast_skip_timer(void)
{
	if (fast_skip_enable)
	{
		if (fast_skip_counter < 0xFFFF)
		{
			++fast_skip_counter;
		}
	}
	else
	{
		fast_skip_counter = 0;
	}
}
