/*
 * date.h
 *
 * Created: 21/09/2017 23:56
 *  Author: josefe
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DATE_H_
#define DATE_H_

#define spanish

#ifdef spanish
#define	ENERO 		1
#define FEBRERO 	2
#define MARZO		3
#define ABRIL		4
#define MAYO		5
#define JUNIO		6
#define JULIO		7
#define AGOSTO		8
#define SEPTIEMBRE	9
#define	OCTUBRE		10	
#define NOVIEMBRE	11
#define DICIEMBRE	12

#define LUNES 		1
#define MARTES 		2
#define MIERCOLES	3
#define JUEVES		4
#define VIERNES		5
#define SABADO		6	
#define DOMINGO		7

#endif


 #define TRUE 1
 #define FALSE 0

 #define A_GREATER_B 1
 #define A_LOWER_B 	-1
 #define A_EQUAL_B   0
 
 #define MONTHS_OF_YEAR 12
 #define DAYS_OF_YEAR 365
 #define DAYS_OF_LEAP_YEAR 366
 #define DAYS_OF_WEEK 7
 #define HOURS_OF_DAY 24 
 #define HOURS_OF_WEEK 168
 #define MINUTES_OF_HOUR 60
 #define MINUTES_OF_DAY 1440
 #define SECONDS_OF_MINUTE 60
 #define SECONDS_OF_HOUR 3600
 #define SECONDS_OF_DAY	86400
 
 #define LEAP_YEAR_MAIN_PERIOD 4 
 #define NON_LEAP_YEAR_PERIOD 100
 #define LEAP_YEAR_EXTRA_PERIOD 400
 
extern const unsigned char daysOfMonths[12];
extern const unsigned char daysOfMonths_leapYear[12];

//TODO
//Add data_type field
typedef struct
{
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char dayW;
	unsigned char dayM;
	unsigned char month;
	unsigned int  year;	
}time_data;

extern const time_data time_reference;

time_data time_init (unsigned char sec,	unsigned char min,	unsigned char hour,	unsigned char dayM,	unsigned char month, unsigned int year);
time_data hour_increment(time_data time);
time_data minute_increment(time_data time);
time_data hour_decrement(time_data time);
time_data minute_decrement(time_data time);

unsigned char is_leap_year(time_data date);
time_data getDayOfWeek (time_data date); //Computes the dayOfWeek from the passed date and overrides it
char* getDayOfWeekSpanishNameUppercase8Char(time_data date); //Prints day of week name

signed char compare_dates_full(time_data date_a, time_data date_b);
signed char compare_dates_rude(time_data date_a, time_data date_b);




#endif 

#ifdef __cplusplus
}
#endif
