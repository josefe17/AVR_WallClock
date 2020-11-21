/*
 * rtc.c
 *
 * Created: 23/10/2017 12:42:06
 *  Author: josefe
 */ 

#include "data_casting.h"
#include "date.h"
#include "rtc.h"
#include "TWI_Master.h"

#define I2C_READ	1
#define I2C_WRITE	0

/*I2C registers addresses*/
#define CLOCK_ADDRESS		0x00
#define ALARM1_ADDRESS		0x07
#define ALARM2_ADDRESS		0x0B
#define CONTROL1_ADDRESS	0x0E
#define CONTROL2_ADDRESS	0x0F
#define AGING_ADDRESS		0x10
#define TEMPERATURE_ADDRESS	0x11

/*I2C Message buffer's sizes*/
#define POINTER_BUFFER_SIZE			2
#define CLOCK_WRITE_BUFFER_SIZE		9
#define CLOCK_READ_BUFFER_SIZE		8
#define ALARM1_WRITE_BUFFER_SIZE	6
#define ALARM1_READ_BUFFER_SIZE		5
#define ALARM2_WRITE_BUFFER_SIZE	5
#define ALARM2_READ_BUFFER_SIZE		4
#define SINGLE_CONTROL_BUFFER_SIZE	3
#define DOUBLE_CONTROL_BUFFER_SIZE	4
#define TEMPERATURE_BUFFER_SIZE		4
#define CONTROL_2_TEMP_BUFFER_SIZE	7 //Non RTC and alarm registers

/*Control masks*/
#define OSCILLATOR_FAULT_MASK		0b10000000
#define OSCILLATOR_FAULT_CLEAR_MASK	0b00001111
#define OSCILLATOR_ENABLE_MASK		0b01111111
#define OSCILLATOR_DISABLE_MASK		0b10000000
#define TEMP_BSY_MASK				0b00000100
#define TEMP_CONV_MASK				0b00100000
#define BATTERY_SQUARE_ENABLE_MASK	0b01000000
#define _32768HZ_ENABLE_MASK		0b00001000


/*Gets current timestamp from rtc unit*/
/*Needs slave address and the data output format*/
/*Returns a time_data with the required timestamp*/
time_data retrieve_timestamp_from_RTC(unsigned char i2c_address, unsigned char data_mode)
{
	unsigned char msg_buff[CLOCK_WRITE_BUFFER_SIZE];
	time_data clock;
	//Set pointer to clock register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CLOCK_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_READ_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, CLOCK_READ_BUFFER_SIZE);
	//arrange data
	if(data_mode==DATA_DECIMAL) //If format is decimal, we cast data
	{		
		clock.sec=bcd2dec(msg_buff[1]);
		clock.min=bcd2dec(msg_buff[2]);
		clock.hour=bcd2dec(msg_buff[3]);
		clock.dayW=msg_buff[4];
		clock.dayM=bcd2dec(msg_buff[5]);
		clock.month=bcd2dec(msg_buff[6] & MONTH_MASK); //removes century information		
		clock.year=2000+((unsigned int) bcd2dec(msg_buff[7])); //Converts year into natural value
		if (msg_buff[6]&CENTURY_MASK) //Adds century bit information
		{
			clock.year-=100;
		}
		return clock;
	}
	if(data_mode==DATA_BCD) //if BCD, data is already formatted
	{		
		clock.sec=msg_buff[1];
		clock.min=msg_buff[2];
		clock.hour=msg_buff[3];
		clock.dayW=msg_buff[4];
		clock.dayM=msg_buff[5];
		clock.month=msg_buff[6] & MONTH_MASK; //removes century information		
		if (msg_buff[6]&CENTURY_MASK)	//Casts year into 4 digit BCD number with century bit information already merged
		{
			clock.year=(unsigned int) (0x1900 | msg_buff[7]);
		}
		else
		{
			clock.year=(unsigned int) (0x2000 | msg_buff[7]); 
		}			
		return clock;
	}
	return clock;	//Undefined time_data if wrong data format is provided (e.g. char)
	
}

/*Configures RTC clock and calendar*/
/*It requires a time_data structure with the time to be setted and its format*/
void update_timestamp_to_RTC(unsigned char i2c_address, time_data time, unsigned char data_mode)
{
		unsigned char msg_buff[CLOCK_WRITE_BUFFER_SIZE];
		
		//Set pointer to clock register
		msg_buff[0]=i2c_address;
		msg_buff[1]=CLOCK_ADDRESS;
		//Formats I2C frame with data
		if (data_mode==DATA_DECIMAL)//If decimal data is cast to BCD and masked not to affect additional flags on time registers
		{
			msg_buff[2]=dec2bcd(time.sec);
			msg_buff[3]=dec2bcd(time.min);
			msg_buff[4]=dec2bcd(time.hour)&CLOCK_24H_MASK; //RTC is hardcoded to work on 24h mode. Display can be changed by SW.
			msg_buff[5]=time.dayW;
			msg_buff[6]=dec2bcd(time.dayM);
			if (time.year<2000) //Century bit set (1 is '1900' century [1900-1999])
			{
				msg_buff[7]=(dec2bcd(time.month) & MONTH_MASK) | CENTURY_MASK;
			}
			else
			{
				msg_buff[7]=dec2bcd(time.month) & MONTH_MASK;
			}
			
			msg_buff[8]=dec2bcd((unsigned char) (time.year%100)); 
			TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
			return;
		}
		if (data_mode==DATA_BCD) //If BCD data is just masked
		{
			msg_buff[2]=(time.sec);
			msg_buff[3]=(time.min);
			msg_buff[4]=(time.hour)&CLOCK_24H_MASK;
			msg_buff[5]=time.dayW;
			msg_buff[6]=(time.dayM); 
			msg_buff[7]=((time.month) & MONTH_MASK) | ((time.year>>1)&CENTURY_MASK); // BCD 9th bit (first upper byte bit) stores century information
			msg_buff[8]=((unsigned char)time.year); //Expected to get just the lower byte
			TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
			return;
		}
}

unsigned char check_oscillator_fault(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];	
	//Set pointer to clock register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	return ((msg_buff[1] & OSCILLATOR_FAULT_MASK)==OSCILLATOR_FAULT_MASK); //1 if oscillator was stopped	
}

unsigned char clear_oscillator_fault(unsigned char i2c_address) //RMW Atomic
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	unsigned char control2_backup;
	//Set pointer to clock register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE); 
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	control2_backup=msg_buff[1];
	//Clear flag
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	msg_buff[2]=(OSCILLATOR_FAULT_CLEAR_MASK & control2_backup) | ALARMS; //A2F and A1F are written to 1 and therefore not cleared
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE); 
	//Return data
	return ((control2_backup & OSCILLATOR_FAULT_MASK)==OSCILLATOR_FAULT_MASK); //1 if oscillator was stopped
}


#ifdef _USE_OSCILLATOR_DISABLE
void enable_oscillator(unsigned char i2c_address) //RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];	
	//Set pointer to clock register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);	
	//Clear bit
	msg_buff[0]=i2c_address;
	msg_buff[2]=(OSCILLATOR_ENABLE_MASK & msg_buff[1]); //0 means oscillator on
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}

void disable_oscillator(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to clock register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Set bit
	msg_buff[0]=i2c_address;
	msg_buff[2]=(OSCILLATOR_DISABLE_MASK | msg_buff[1]); //1 means oscillator off
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}
#endif

#ifdef _USE_ALARM1
/*Get alarm1 information from RTC*/
/*Needs slave address, data format and returs the time_data with the values*/
time_data retrieve_alarm1_from_RTC(unsigned char i2c_address, unsigned char data_mode)
{
	unsigned char msg_buff[ALARM1_WRITE_BUFFER_SIZE];
	time_data alarm1_time;
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=ALARM1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, ALARM1_READ_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, ALARM1_READ_BUFFER_SIZE);
	//arrange data
	if(data_mode==DATA_DECIMAL)
	{
		alarm1_time.sec=bcd2dec(msg_buff[1] & SEC_MIN_HOUR_MASK);
		alarm1_time.min=bcd2dec(msg_buff[2] & SEC_MIN_HOUR_MASK);
		alarm1_time.hour=bcd2dec(msg_buff[3] & CLOCK_24H_MASK);
		if(msg_buff[4]&ALARM_DAY_MODE_MASK) //If true, this byte stores data about day of the week
		{
			alarm1_time.dayW=msg_buff[4]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK; //Masked to remove mode overhead
		}
		else
		{
			alarm1_time.dayM=bcd2dec(msg_buff[4]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK);
		}				
		return alarm1_time;
	}
	if(data_mode==DATA_BCD)
	{
		alarm1_time.sec=msg_buff[1] & SEC_MIN_HOUR_MASK;
		alarm1_time.min=msg_buff[2] & SEC_MIN_HOUR_MASK;
		alarm1_time.hour=msg_buff[3]& CLOCK_24H_MASK;
		if(msg_buff[4]&ALARM_DAY_MODE_MASK) //If true, this byte stores data about day of the week
		{
			alarm1_time.dayW=msg_buff[4]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK; //Masked to remove mode overhead
		}
		else
		{
			alarm1_time.dayM=msg_buff[4]& ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK;
		}
		return alarm1_time;		
	}
	return alarm1_time;

}

void update_alarm1_to_RTC(unsigned char i2c_address, time_data alarm1_time, unsigned char data_mode, unsigned char alarm1_mask)
{
	unsigned char msg_buff[ALARM1_WRITE_BUFFER_SIZE];
	
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=ALARM1_ADDRESS;
	if (data_mode==DATA_DECIMAL)
	{
		msg_buff[2]=((alarm1_mask<<7)&ALARM_MODE_BIT_MASK) | dec2bcd(alarm1_time.sec);
		msg_buff[3]=((alarm1_mask<<6)&ALARM_MODE_BIT_MASK) | dec2bcd(alarm1_time.min);
		msg_buff[4]=((alarm1_mask<<5)&ALARM_MODE_BIT_MASK) | (dec2bcd(alarm1_time.hour)& ALARM_24H_MASK);
		if (alarm1_mask & ALARM_DAY_MODE_MASK)
		{
			msg_buff[5]=((alarm1_mask<<4)&ALARM_MODE_BIT_MASK) | ALARM_DAY_MODE_MASK | alarm1_time.dayW;
		}
		else
		{
			msg_buff[5]=((alarm1_mask<<4)&ALARM_MODE_BIT_MASK) | (dec2bcd(alarm1_time.dayM));
		}
		
		TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
		return;
	}
	if (data_mode==DATA_BCD)
	{
		msg_buff[2]=(((alarm1_mask<<7)&ALARM_MODE_BIT_MASK) | alarm1_time.sec);
		msg_buff[3]=(((alarm1_mask<<6)&ALARM_MODE_BIT_MASK) | alarm1_time.min);
		msg_buff[4]=(((alarm1_mask<<5)&ALARM_MODE_BIT_MASK) | alarm1_time.hour)&CLOCK_24H_MASK;
		if (alarm1_mask & ALARM_DAY_MODE_MASK)
		{
			msg_buff[5]=((alarm1_mask<<4)&ALARM_MODE_BIT_MASK) | ALARM_DAY_MODE_MASK | alarm1_time.dayW;
		}
		else
		{
			msg_buff[5]=((alarm1_mask<<4)&ALARM_MODE_BIT_MASK) | (alarm1_time.dayM);
		}
		TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
		return;
	}
}

unsigned char get_alarm1_mode(unsigned char i2c_address)
{
		unsigned char msg_buff[ALARM1_WRITE_BUFFER_SIZE];		
		//Set pointer to alarm register
		msg_buff[0]=i2c_address;
		msg_buff[1]=ALARM1_ADDRESS;
		TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
		//Retrieve_data
		msg_buff[0]=i2c_address|I2C_READ;
		TWI_Start_Transceiver_With_Data(msg_buff, ALARM1_READ_BUFFER_SIZE);
		TWI_Get_Data_From_Transceiver(msg_buff, ALARM1_READ_BUFFER_SIZE);
		//Arrange data
		return (((msg_buff[1]&ALARM_MODE_BIT_MASK)>>7) | ((msg_buff[2]&ALARM_MODE_BIT_MASK)>>6) | ((msg_buff[3]&ALARM_MODE_BIT_MASK)>>5) | ((msg_buff[4]&ALARM_MODE_BIT_MASK)>>4) | ((msg_buff[4]&ALARM_DAY_MODE_MASK)>>2));
}
#endif


#ifdef _USE_ALARM2
time_data retrieve_alarm2_from_RTC(unsigned char i2c_address, unsigned char data_mode)
{
	unsigned char msg_buff[ALARM2_WRITE_BUFFER_SIZE];
	time_data alarm2_time;
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=ALARM2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, ALARM2_READ_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, ALARM2_READ_BUFFER_SIZE);
	//arrange data
	if(data_mode==DATA_DECIMAL)
	{
		alarm2_time.sec=0;
		alarm2_time.min=bcd2dec(msg_buff[1] & SEC_MIN_HOUR_MASK);
		alarm2_time.hour=bcd2dec(msg_buff[2] & CLOCK_24H_MASK);
		if(msg_buff[3]&ALARM_DAY_MODE_MASK) //If true, this byte stores data about day of the week
		{
			alarm2_time.dayW=msg_buff[3]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK; //Masked to remove mode overhead
		}
		else
		{
			alarm2_time.dayM=bcd2dec(msg_buff[3]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK);
		}
		return alarm2_time;
	}
	if(data_mode==DATA_BCD)
	{
		alarm2_time.sec=0;
		alarm2_time.min=msg_buff[1] & SEC_MIN_HOUR_MASK;
		alarm2_time.hour=msg_buff[2]& CLOCK_24H_MASK;
		if(msg_buff[3]&ALARM_DAY_MODE_MASK) //If true, this byte stores data about day of the week
		{
			alarm2_time.dayW=msg_buff[3]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK; //Masked to remove mode overhead
		}
		else
		{
			alarm2_time.dayM=msg_buff[3]&ALARM_DAY_MODE_MASK & ALARM_MODE_BIT_MASK;
		}
		return alarm2_time;
	}
	return alarm2_time;
}

void update_alarm2_to_RTC(unsigned char i2c_address, time_data alarm2_time, unsigned char data_mode, unsigned char alarm2_mask)
{
	unsigned char msg_buff[ALARM2_WRITE_BUFFER_SIZE];
	
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=ALARM2_ADDRESS;
	if (data_mode==DATA_DECIMAL)
	{
		msg_buff[2]=((alarm2_mask<<6)&ALARM_MODE_BIT_MASK) | dec2bcd(alarm2_time.min);
		msg_buff[3]=((alarm2_mask<<5)&ALARM_MODE_BIT_MASK) | (dec2bcd(alarm2_time.hour)& ALARM_24H_MASK);
		if (alarm2_mask & ALARM_DAY_MODE_MASK)
		{
			msg_buff[4]=((alarm2_mask<<4)&ALARM_MODE_BIT_MASK) | ALARM_DAY_MODE_MASK | alarm2_time.dayW;
		}
		else
		{
			msg_buff[4]=((alarm2_mask<<4)&ALARM_MODE_BIT_MASK) | (dec2bcd(alarm2_time.dayM));
		}
		
		TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
		return;
	}
	if (data_mode==DATA_BCD)
	{		
		msg_buff[2]=(((alarm2_mask<<6)&ALARM_MODE_BIT_MASK) | alarm2_time.min);
		msg_buff[3]=(((alarm2_mask<<5)&ALARM_MODE_BIT_MASK) | alarm2_time.hour)&CLOCK_24H_MASK;
		if (alarm2_mask & ALARM_DAY_MODE_MASK)
		{
			msg_buff[4]=((alarm2_mask<<4)&ALARM_MODE_BIT_MASK) | ALARM_DAY_MODE_MASK | alarm2_time.dayW;
		}
		else
		{
			msg_buff[4]=((alarm2_mask<<4)&ALARM_MODE_BIT_MASK) | (alarm2_time.dayM);
		}
		TWI_Start_Transceiver_With_Data(msg_buff, CLOCK_WRITE_BUFFER_SIZE);
		return;
	}
}

unsigned char get_alarm2_mode(unsigned char i2c_address)
{
	unsigned char msg_buff[ALARM2_WRITE_BUFFER_SIZE];
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=ALARM2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, ALARM2_READ_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, ALARM2_READ_BUFFER_SIZE);
	//Arrange data
	return (((msg_buff[1]&ALARM_MODE_BIT_MASK)>>6) | ((msg_buff[2]&ALARM_MODE_BIT_MASK)>>5) | ((msg_buff[3]&ALARM_MODE_BIT_MASK)>>4) | ((msg_buff[3]&ALARM_DAY_MODE_MASK)>>2));
}
#endif


#if defined(_USE_ALARM1) || defined(_USE_ALARM2)
unsigned char get_alarm_enable(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	return (msg_buff[1]& ALARMS);
}
void update_alarms_enable(unsigned char i2c_address, unsigned char alarm1_status, unsigned char alarm2_status)//RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(NOT_ALARMS & msg_buff[1]); //Other bits are left as is
	switch (alarm1_status) //Configure alarm1
	{
		case ALARM_NOT_MODFY: //Not modify alarm1 enable bit
		msg_buff[2]|=(msg_buff[1]&ALARM1);
		break;
		case ALARM_ENABLE: //Set alarm1 enable bit
		msg_buff[2]|=ALARM1;
		break;
		case ALARM_DISABLE: //0 or unknown value turns alarm off
		default:
		break;
	}
	switch (alarm2_status) //Configure alarm2
	{
		case ALARM_NOT_MODFY: // Not modify alarm2 enable bit
		msg_buff[2]|=(msg_buff[1]&ALARM2);
		break;
		case ALARM_ENABLE: //Set alarm2 enable bit
		msg_buff[2]|=ALARM2;
		break;
		case ALARM_DISABLE://0 or unknown value turns alarm off
		default:
		break;
	}
	msg_buff[1]=CONTROL1_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}

unsigned char get_alarm_flags(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	return (msg_buff[1]& ALARMS);
}
void clear_alarm_flags(unsigned char i2c_address, unsigned char alarm_flags_mask)//RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to alarm register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(NOT_ALARMS & msg_buff[1]) | (alarm_flags_mask & ALARMS); //Other bits are left as is	
	msg_buff[1]=CONTROL2_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}
#endif

#ifdef _USE_TEMP_SENSOR
signed int get_temperature(unsigned char i2c_address)
{
	unsigned char msg_buff[TEMPERATURE_BUFFER_SIZE];
	//Set pointer to temperature register
	msg_buff[0]=i2c_address;
	msg_buff[1]=TEMPERATURE_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, TEMPERATURE_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, TEMPERATURE_BUFFER_SIZE);
	return  (msg_buff[2] | (msg_buff[1]<<8));
}

unsigned char get_temperature_status(unsigned char i2c_address)
{
	unsigned char msg_buff[DOUBLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, DOUBLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, DOUBLE_CONTROL_BUFFER_SIZE);
	if(msg_buff[1]&TEMP_CONV_MASK)  //If is set, an user conversion has been triggede
	{
		return TEMP_USER_CONVERSION;
	}
	if (msg_buff[2]&TEMP_BSY_MASK) //And if CONV is 0 but BSY is set, a system conversion is being done.
	{
		return TEMP_ONGOING_CONVERSION;
	}
	return TEMP_IDLE;
}

void trig_temp_conversion(unsigned char i2c_address) //RMW ATOMIC
{
	unsigned char msg_buff[DOUBLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, DOUBLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, DOUBLE_CONTROL_BUFFER_SIZE);
	if((!(msg_buff[1]&TEMP_CONV_MASK)) && (!(msg_buff[2]&TEMP_BSY_MASK))) //If sensor is idle
	{
		msg_buff[0]=i2c_address;
		msg_buff[2]= msg_buff[1] | TEMP_CONV_MASK; //Configure for a new conversion
		msg_buff[1]=CONTROL1_ADDRESS;
		TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	} 
}
#endif

#ifdef _USE_CONFIG_SQUARE_INT_PIN
void set_square_freq(unsigned char i2c_address, unsigned char freq_mask) //RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;	
	msg_buff[2]=(msg_buff[1]&(~SQUARE_8K192HZ))|(freq_mask&SQUARE_8K192HZ); //other bits as is	
	msg_buff[1]=CONTROL1_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}

void configure_square_int_pin(unsigned char i2c_address, unsigned char pinMode)//RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(msg_buff[1]&(~PINMODE_ALARM_INTERRUPT))|(pinMode&PINMODE_ALARM_INTERRUPT); //other bits as is
	msg_buff[1]=CONTROL1_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}

void set_square_battery(unsigned char i2c_address) //RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(msg_buff[1]&(~BATTERY_SQUARE_ENABLE_MASK))|BATTERY_SQUARE_ENABLE_MASK; //other bits as is
	msg_buff[1]=CONTROL1_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}
void reset_square_battery(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL1_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(msg_buff[1]&(~BATTERY_SQUARE_ENABLE_MASK)); //other bits as is
	msg_buff[1]=CONTROL1_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}
#endif

#ifdef _USE_CONFIG_32K_PIN
void set_32K_freq(unsigned char i2c_address) //RMW ATOMIC
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(msg_buff[1]&(~_32768HZ_ENABLE_MASK))|_32768HZ_ENABLE_MASK; //other bits as is
	msg_buff[1]=CONTROL2_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}

void reset_32K_freq(unsigned char i2c_address)
{
	unsigned char msg_buff[SINGLE_CONTROL_BUFFER_SIZE];
	//Set pointer to control1 register
	msg_buff[0]=i2c_address;
	msg_buff[1]=CONTROL2_ADDRESS;
	TWI_Start_Transceiver_With_Data(msg_buff, POINTER_BUFFER_SIZE);
	//Retrieve_data
	msg_buff[0]=i2c_address|I2C_READ;
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	TWI_Get_Data_From_Transceiver(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
	//Configure alarms
	msg_buff[0]=i2c_address;
	msg_buff[2]=(msg_buff[1]&(~_32768HZ_ENABLE_MASK)); //other bits as is
	msg_buff[1]=CONTROL2_ADDRESS;
	//Send data
	TWI_Start_Transceiver_With_Data(msg_buff, SINGLE_CONTROL_BUFFER_SIZE);
}
#endif