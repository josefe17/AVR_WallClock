#include "date.h"

const unsigned char daysOfMonths[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const unsigned char daysOfMonths_leapYear[12]={31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const time_data time_reference = {0, 0, 0, SABADO, 1, ENERO, 2000};

time_data time_init (unsigned char sec,	unsigned char min,	unsigned char hour,	unsigned char dayM,	unsigned char month, unsigned int year)
{
	time_data time;
	time.year=year; //Years aren't checked
	
	if (sec>=SECONDS_OF_MINUTE) //Second checking
	{
		sec=0;
	}
	time.sec=sec;
	
	if (min>=MINUTES_OF_HOUR) //Minutes checking
	{
		min=0;
	}
	time.min=min;
	
	if (month == 0 || (month > MONTHS_OF_YEAR)) //Month checking
	{
		month = ENERO;
	}
	time.month=month;
	
	if (is_leap_year(time)) //Day of month checking with leap year correction
	{
		if ( dayM==0 || (dayM>daysOfMonths_leapYear[month-1]))
		{
			dayM=1;
		}
	}
	else
	{
		if ( dayM==0 || (dayM>daysOfMonths[month-1]))
		{
			dayM=1;
		}
	}
	time.dayM=dayM; 
	
	return getDayOfWeek(time); //Day of week calculation
	
}

unsigned char is_leap_year(time_data date)
{
	if (date.year % LEAP_YEAR_EXTRA_PERIOD) //If year isn't 400x
	{
		if (date.year % NON_LEAP_YEAR_PERIOD) //If year isn't 100x and 400x
		{
			if (date.year % LEAP_YEAR_MAIN_PERIOD) //If year isn't 4x, 100x and 400x
			{
				return FALSE; //Isn't leap year
			}
			return TRUE; //Isn't 100x and 400x, but it is 4x
		}
		return FALSE; //Isn't 400x but it is 100x
	}
	return TRUE; //Is 400x
}
	
	
time_data getDayOfWeek (time_data date)
{
  const unsigned char month_beginning_offset[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};  
  unsigned int year_copy = date.year;  
  year_copy -= (date.month < MARZO);
  date.dayW= (unsigned char)((year_copy + year_copy/LEAP_YEAR_MAIN_PERIOD - year_copy/NON_LEAP_YEAR_PERIOD + year_copy/LEAP_YEAR_EXTRA_PERIOD + month_beginning_offset[date.month-1] + date.dayM-1) % DAYS_OF_WEEK)+1;
  return date;
}

char* getDayOfWeekSpanishNameUppercase8Char(time_data date)
{
	switch (date.dayW)
	{
		case LUNES:
			return "LUNES   ";
		case MARTES:
			return "MARTES  ";
		case MIERCOLES:
			return "MIERCOLE"; //Opps longer
		case JUEVES:
			return "JUEVES  ";
		case VIERNES:
			return "VIERNES ";
		case SABADO:
			return "SABADO  ";
		case DOMINGO:
			return "DOMINGO ";
		default:
			return "        ";
	}
}


	
time_data hour_increment(time_data time)
{
	++time.hour;
	if (time.hour >=HOURS_OF_DAY)
	{
		time.hour = 0;
		++time.dayM;
		//time.dayW=((time.dayW)%DAYS_OF_WEEK)+1; 
		time.dayW=(time.dayW>=DAYS_OF_WEEK)?LUNES:time.dayW+1;
		if (is_leap_year(time)) //Day of month checking with leap year correction
		{
			if (time.dayM>daysOfMonths_leapYear[time.month-1])
			{
				time.dayM=1;
				++time.month;
				if (time.month > MONTHS_OF_YEAR) 
				{
					time.month = ENERO;
					++time.year;
				}
				
				
			}
		}	
		else
		{
			if  (time.dayM>daysOfMonths[time.month-1])
			{
				time.dayM=1;
				++time.month;
				if (time.month > MONTHS_OF_YEAR) 
				{
					time.month = ENERO;
					++time.year;
				}
			}
		}
	}
	return time;
}

time_data minute_increment(time_data time)
{
	++time.min;
	if (time.min>=MINUTES_OF_HOUR)
	{
		time.min=0;
		return hour_increment(time);
	}
	return time;
}

time_data hour_decrement(time_data time)
{	
	if (time.hour == 0) //If hour is 00 am
	{
		time.hour = HOURS_OF_DAY-1; //goes to 23 pm
		
		if (time.dayM<=1) //If the current day is first one of the month
		{
			if(time.month<=ENERO) //If it is New Year
			{
				time.month=DICIEMBRE;	//Previous month is December			
				--time.year;			//Decrement year
			}
			else
			{
				--time.month;
			}
			
			if (is_leap_year(time)) //if it is a leap year
			{
				time.dayM=daysOfMonths_leapYear[time.month-1]; //Current day is the last one of the previous month, even leap
			}
			else
			{
				time.dayM=daysOfMonths[time.month-1]; //Current day is the last one of the previous month
			}
			time.dayW=(time.dayW<=LUNES)?DOMINGO:time.dayW-1; //Recalculates day of week
			return time;
		}
		--time.dayM;
		time.dayW=(time.dayW<=LUNES)?DOMINGO:time.dayW-1; //Recalculates day of week
		return time;		
	}	
	--time.hour;
	return time;
}

time_data minute_decrement(time_data time)
{	
	if (time.min<=0)
	{
		time.min=MINUTES_OF_HOUR-1;
		return hour_decrement(time);
	}
	--time.min;
	return time;
}

signed char compare_dates_rude(time_data date_a, time_data date_b)
{
	//Checks months
	if (date_a.month>date_b.month)
	{
		return A_GREATER_B;
	}
		if (date_a.month<date_b.month)
	{
		return A_LOWER_B;
	}
	//Checks days
	if (date_a.dayM>date_b.dayM)
	{
		return A_GREATER_B;
	}
	if (date_a.dayM<date_b.dayM)
	{
		return A_LOWER_B;
	}
	//Checks hours
	if (date_a.hour>date_b.hour)
	{
		return A_GREATER_B;
	}
		if (date_a.hour<date_b.hour)
	{
		return A_LOWER_B;
	}
	//Check minutes
	if (date_a.min>date_b.min)
	{
		return A_GREATER_B;
	}
	if (date_a.min<date_b.min)
	{
		return A_LOWER_B;
	}
	//Check seconds
	if (date_a.sec>date_b.sec)
	{
		return A_GREATER_B;
	}
	if (date_a.sec<date_b.sec)
	{
		return A_LOWER_B;
	}
	return A_EQUAL_B; //Equal
}

signed char compare_dates_full(time_data date_a, time_data date_b)
{
	//Checks years
	if (date_a.year>date_b.year)
	{
		return A_GREATER_B;
	}
		if (date_a.year<date_b.year)
	{
		return A_LOWER_B;
	}
	//Checks months
	if (date_a.month>date_b.month)
	{
		return A_GREATER_B;
	}
		if (date_a.month<date_b.month)
	{
		return A_LOWER_B;
	}
	//Checks days
	if (date_a.dayM>date_b.dayM)
	{
		return A_GREATER_B;
	}
	if (date_a.dayM<date_b.dayM)
	{
		return A_LOWER_B;
	}
	//Checks hours
	if (date_a.hour>date_b.hour)
	{
		return A_GREATER_B;
	}
		if (date_a.hour<date_b.hour)
	{
		return A_LOWER_B;
	}
	//Check minutes
	if (date_a.min>date_b.min)
	{
		return A_GREATER_B;
	}
	if (date_a.min<date_b.min)
	{
		return A_LOWER_B;
	}
	//Check seconds
	if (date_a.sec>date_b.sec)
	{
		return A_GREATER_B;
	}
	if (date_a.sec<date_b.sec)
	{
		return A_LOWER_B;
	}
	return A_EQUAL_B; //Equal
}

	

	
	

