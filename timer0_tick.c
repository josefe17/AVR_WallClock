/*
 * timer0_tick.c
 *
 * Created: 01/11/2017 12:05:01
 *  Author: josefe
 */ 

#include "timer0_tick.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile unsigned int interrupts_count;
volatile unsigned int interrupts_counter;

void timer0_tick_init(unsigned char prescaler, unsigned char count, unsigned int interrupts_count_value)
{
	interrupts_count=interrupts_count_value;	//Set the number of interrupts to be counted before setting flag (pseudo-post-scaler)
	interrupts_counter=0;						//Clear SW postscaler
	timeout_flag=0;								//Clear timeout flag
	TCCR0A=0b00000010;							//CTC with compare match threshold on OCRA. No HW pin toggling.
	OCR0A=count;								//Set threshold
	TCNT0=0;									//Clear count
	TIFR0|=	0b00000010;							//Clear flag
	TIMSK0|= 0b00000010;						//Interrupt enable	
	TCCR0B=prescaler & 0b00000111;				//Timer on	
}

void delay_until_tick(void)
{
	while(!timeout_flag);
	timeout_flag=0;	
}

ISR (TIMER0_COMPA_vect)
{
	++interrupts_counter;						//Increments post-scaler
	if((interrupts_counter>interrupts_count))	//If threshold is overcome
	{
		++timeout_flag;							//Set flag
		interrupts_counter=0;					//Reset timer
		TCNT0=0;
	}
}
