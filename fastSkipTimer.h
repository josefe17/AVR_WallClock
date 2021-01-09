/*
 * fastSkipTimer.h
 *
 *  Created on: 17 dic. 2020
 *      Author: josefe
 */

#ifndef FASTSKIPTIMER_H_
#define FASTSKIPTIMER_H_


void start_fast_skip_timer(unsigned int timeout);
void stop_fast_skip_timer(void);
unsigned char is_fast_skip_timer_over(void);
void increment_fast_skip_timer(void);


#endif /* FASTSKIPTIMER_H_ */
