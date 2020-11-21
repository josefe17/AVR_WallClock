/*
 * rtc.h
 *
 * Created: 22/10/2017 9:08:20
 *  Author: josefe
 */ 


#ifndef RTC_H_
#define RTC_H_

#include "date.h"

/*Compiling options*/
/*Used to optimize size*/
#define _USE_ALARM1
#define _USE_ALARM2
//#define _USE_OSCILLATOR_DISABLE
#define _USE_TEMP_SENSOR
//#define _USE_CONFIG_SQUARE_INT_PIN
//#define _USE_CONFIG_32K_PIN	

/*Data type choser*/
#define DATA_DECIMAL			0x00
#define DATA_BCD				0xFF
#define DATA_CHAR				0b10101010

/*Clock masks*/
#define CLOCK_24H_MASK			0b00111111
#define ALARM_24H_MASK			0b10111111
#define CENTURY_MASK			0b10000000
#define MONTH_MASK				0b00011111
#define SEC_MIN_HOUR_MASK		0b01111111

/*Year bounds*/
#define MIN_YEAR				1900
#define MAX_YEAR				2099

/*Alarm modes*/
#define ALARM_DAY_MATCH			0b00010000
#define ALARM_DATE_MATCH		0b00000000
#define ALARM_HOUR_MATCH		0b00001000
#define ALARM_MINUTE_MATCH		0b00001100
#define ALARM_SECOND_MATCH		0b00001110
#define ALARM_PERIODIC_MATCH	0b00001111

/*Alarm status and control*/
#define ALARM_ENABLE			1
#define ALARM_DISABLE			0
#define ALARM_NOT_MODFY			255
#define ALARM_FIRED				255
#define ALARM_IDLE				0

/*Alarm masks*/
#define ALARM1					1
#define ALARM2					2
#define ALARMS					3
#define NOT_ALARM1				0b11111110
#define NOT_ALARM2				0b11111101
#define NOT_ALARMS				0b11111100
#define ALARM_MODE_BIT_MASK		0b10000000
#define ALARM_DAY_MODE_MASK		0b01000000

/*Temperature sensor*/
#define TEMP_ONGOING_CONVERSION 0x0F
#define TEMP_USER_CONVERSION	0xFF
#define TEMP_IDLE				0x0	

/*Square wave-interrupt pin*/
#define PINMODE_ALARM_INTERRUPT	0b00000100
#define PINMODE_SQUARE_WAVE		0b00000000	
#define SQUARE_1HZ				0b00000000
#define SQUARE_1K024HZ			0b00001000
#define SQUARE_4K096HZ			0b00010000
#define SQUARE_8K192HZ			0b00011000

/*Prototypes*/
//TODO
//Add data_type parameter

time_data retrieve_timestamp_from_RTC(unsigned char i2c_address, unsigned char data_mode);
void update_timestamp_to_RTC(unsigned char i2c_address, time_data time, unsigned char data_mode);

unsigned char check_oscillator_fault(unsigned char i2c_address);
unsigned char clear_oscillator_fault(unsigned char i2c_address); //RMW Atomic


#ifdef _USE_OSCILLATOR_DISABLE
void enable_oscillator(unsigned char i2c_address); //RMW ATOMIC
void disable_oscillator(unsigned char i2c_address);
#endif

#ifdef _USE_ALARM1
time_data retrieve_alarm1_from_RTC(unsigned char i2c_address, unsigned char data_mode);
void update_alarm1_to_RTC(unsigned char i2c_address, time_data alarm1_time, unsigned char data_mode, unsigned char alarm1_mask);
unsigned char get_alarm1_mode(unsigned char i2c_address);
#endif

#ifdef _USE_ALARM2
time_data retrieve_alarm2_from_RTC(unsigned char i2c_address, unsigned char data_mode);
void update_alarm2_to_RTC(unsigned char i2c_address, time_data alarm2_time, unsigned char data_mode, unsigned char alarm2_mask);
unsigned char get_alarm2_mode(unsigned char i2c_address);
#endif

#if defined(_USE_ALARM1) || defined(_USE_ALARM2)
unsigned char get_alarm_enable(unsigned char i2c_address);
void update_alarms_enable(unsigned char i2c_address, unsigned char alarm1_status, unsigned char alarm2_status);//RMW ATOMIC
unsigned char get_alarm_flags(unsigned char i2c_address);
void clear_alarm_flags(unsigned char i2c_address, unsigned char alarm_flags_mask);//RMW ATOMIC
#endif // _USE_ALARM1 || _USE_ALARM2

//void update_all_to_RTC(unsigned char i2c_address, time_data time, time_data alarm1_time, time_data alarm2_time);

#ifdef _USE_TEMP_SENSOR
signed int get_temperature(unsigned char i2c_address);
unsigned char get_temperature_status(unsigned char i2c_address);
void trig_temp_conversion(unsigned char i2c_address); //RMW ATOMIC
#endif

#ifdef _USE_CONFIG_SQUARE_INT_PIN
void set_square_freq(unsigned char i2c_address, unsigned char freq_mask); //RMW ATOMIC
void configure_square_int_pin(unsigned char i2c_address, unsigned char pinMode);//RMW ATOMIC
void set_square_battery(unsigned char i2c_address); //RMW ATOMIC
void reset_square_battery(unsigned char i2c_address);
#endif

#ifdef _USE_CONFIG_32K_PIN
void set_32K_freq(unsigned char i2c_address); //RMW ATOMIC
void reset_32K_freq(unsigned char i2c_address);
#endif


#endif /* RTC_H_ */