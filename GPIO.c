/*
 * GPIO.c
 *
 * Created: 01/11/2017 14:28:54
 *  Author: josefe
 */ 

#include "GPIO.h"
#include <avr/io.h>

void gpio_init(void)
{
	button_flags=0;
	
	//Enable pull up
	MCUCR&=~(1<<PUD);		
	
	//Set directions
	DDRB|=(GPIO_DDRB_MASK & GPIO_PORTB_MASK);
	DDRC|=(GPIO_DDRC_MASK & GPIO_PORTC_MASK);
	DDRD|=(GPIO_DDRD_MASK & GPIO_PORTD_MASK);
		
	//Enable local pullup or set reset level
	PORTB|=(PORTB_RESET_VALUE & GPIO_PORTB_MASK);	
	PORTC|=(PORTC_RESET_VALUE & GPIO_PORTC_MASK);
	PORTD|=(PORTD_RESET_VALUE & GPIO_PORTD_MASK);
}

void update_button_flags(void)
{
	button_flags=(((~PINC) & GPIO_PORTC_MASK)<<8) | ((~PIND) & GPIO_PORTD_MASK);
}


