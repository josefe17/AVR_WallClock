/*
 * timer0_tick.h
 *
 * Created: 01/11/2017 12:03:13
 *  Author: josefe
 */ 


#ifndef TIMER0_TICK_H_
#define TIMER0_TICK_H_

#define T0_DISABLED				0			
#define T0_PRESCALER_DISABLED	0b00000001
#define T0_PRESCALER_8			0b00000010
#define T0_PRESCALER_64			0b00000011
#define T0_PRESCALER_256		0b00000100
#define T0_PRESCALER_1024		0b00000101
#define T0_EXTERNAL_FALLING		0b00000110
#define T0_EXTERNAL_RISING		0b00000111

volatile unsigned char timeout_flag;

void timer0_tick_init(unsigned char prescaler, unsigned char count, unsigned int interrupts_count_value);
void delay_until_tick(void);

#endif /* TIMER0_TICK_H_ */
