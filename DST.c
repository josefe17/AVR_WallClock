/*
 * DST.c
 *
 * Created: 04/10/2023 13:21:11
 *  Author: josefe
 */ 

#include "DST.h"
#include <avr/eeprom.h>

DSTData dstData;

unsigned char isCurrentTimeDST(volatile time_data* currentTime);
unsigned char checkDST(volatile time_data* currentTime);
unsigned char updateDSTOnStartup(volatile time_data* currentTime);
unsigned char persistDSTData(void);

unsigned char DSTInit(volatile time_data* currentTime)
{
	eeprom_busy_wait();
	uint8_t raw =  eeprom_read_byte((uint8_t*) DST_EEPROM_ADDRESS);	
	dstData.dstRegion = (DSTRegions) ((raw >> 7) & 1);
	dstData.dstEnabled = ((raw >> 6) & 1);
	dstData.dstStatus = ((raw >> 5) & 1);
	dstData.dstForwardHour = (raw & 0b00011111);
	if (updateDSTOnStartup(currentTime))
	{
		return persistDSTData();
	}
	return 1;
}

unsigned char processDST(volatile time_data* currentTime)
{
	if (checkDST(currentTime))
	{
		return persistDSTData();
	}
	return 1;
}

unsigned char enableDST(void)
{
	if (dstData.dstEnabled != 1)
	{
		dstData.dstEnabled = 1;
		return persistDSTData();
	}
	return 1;
}

unsigned char disableDST(void)
{
	if (dstData.dstEnabled != 0)
	{	
		dstData.dstEnabled = 0;
		return persistDSTData();
	}
	return 1;
}

unsigned char getDSTEnabledStatus(void)
{
	return dstData.dstEnabled;
}

char* getDSTNameString(void)
{
	return "dSt ";
}

char* getDSTOnString(void)
{
	return "On  ";
}

char* getDSTOffString(void)
{
	return "Off ";
}

unsigned char setDSTEURegion(void)
{
	if (dstData.dstRegion != DST_EU)
	{
		dstData.dstRegion = DST_EU;
		return persistDSTData();
	}
	return 1;
}

unsigned char setDSTUSARegion(void)
{
	if (dstData.dstRegion != DST_USA)
	{
		dstData.dstRegion = DST_USA;
		return persistDSTData();
	}
	return 1;
}

DSTRegions getDSTRegion(void)
{
	return dstData.dstRegion;
}

char* getDSTRegionCaptionString(void)
{
	return "reg ";
}

char* getDSTEURegionString(void)
{
	return "Euro";
}

char* getDSTUSARegionString(void)
{
	return "EEUU";
}

unsigned char setEUDSTForwardHour (unsigned char hour)
{
	if (dstData.dstForwardHour != (hour & 0b00011111))
	{
		dstData.dstForwardHour = hour & 0b00011111;
		return persistDSTData();
	}
	return 1;
}

unsigned char getEUDSTForwardHour(void)
{
	return dstData.dstForwardHour;
}

char* getDSTForwardHourCaptionString(void)
{
	return "Adel";
}

unsigned char isCurrentTimeDST(volatile time_data* currentTime)
{
	unsigned char currentMonth = currentTime -> month;
	if (dstData.dstRegion == DST_EU)
	{
		if (currentMonth == NOVIEMBRE || currentMonth == DICIEMBRE || currentMonth == ENERO || currentMonth == FEBRERO)
		{
			return 0;
		}
		if (currentMonth == ABRIL || currentMonth == MAYO || currentMonth == JUNIO || currentMonth == JULIO || currentMonth == AGOSTO || currentMonth == SEPTIEMBRE)
		{
			return 1;
		}
		
		if (currentMonth == MARZO)
		{
			if (currentTime -> dayW == DOMINGO && ((DAYS_OF_MARCH - currentTime -> dayM) < 7)) // DST day
			{
				if (currentTime -> hour >= dstData.dstForwardHour)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				if (DAYS_OF_MARCH - currentTime -> dayM + ((currentTime -> dayW) % 7) < 7)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
		if (currentMonth == OCTUBRE)
		{
			if (currentTime -> dayW == DOMINGO && ((DAYS_OF_OCTOBER - currentTime -> dayM) < 7)) // DST day
			{
				if (dstData.dstStatus) // Clock was last turned on during DST time, so RTC timestamp has DST time accounted
				{
					// If we are prior or equal to DST fallback instant (e.g. 3 AM in the natural clock),
					// the RTC time is right and we do not need to do anything (we're still on DST)
					if (currentTime -> hour <= dstData.dstForwardHour + 1)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else // Clock wasn't turned on during DST, so the RTC timestamp is still in non-DST time
				{
					// If we are prior or equal to DST fallback instant, the clock needs to be forwarded 1 h
					// as we are still on DST
					if (currentTime -> hour <= dstData.dstForwardHour )
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				if (DAYS_OF_OCTOBER - currentTime -> dayM + ((currentTime -> dayW) % 7) < 7)
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
		}
	}
	if (dstData.dstRegion == DST_USA)
	{
		if (currentMonth == DICIEMBRE || currentMonth == ENERO || currentMonth == FEBRERO)
		{
			return 0;
		}
		if (currentMonth == ABRIL || currentMonth == MAYO || currentMonth == JUNIO || currentMonth == JULIO || currentMonth == AGOSTO || currentMonth == SEPTIEMBRE || currentMonth == OCTUBRE)
		{
			return 1;
		}
		
		if (currentMonth == MARZO)
		{
			if (currentTime -> dayW == DOMINGO && (currentTime -> dayM >= 8) && (currentTime -> dayM <= 14)) // DST day
			{
				if (currentTime -> hour >= 2)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				if ((13 + currentTime -> dayM - currentTime -> dayW) / 7 > 2)
				{
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
		if (currentMonth == NOVIEMBRE)
		{
			if (currentTime -> dayW == DOMINGO && (currentTime -> dayM <= 7)) // DST day
			{
				if (dstData.dstStatus) // Clock was last turned on during DST time, so RTC timestamp has DST time accounted
				{
					// If we are prior or equal to DST fallback instant (e.g. 3 AM in the natural clock),
					// the RTC time is right and we do not need to do anything (we're still on DST)
					if (currentTime -> hour <= (2 + 1))
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
				else // Clock wasn't turned on during DST, so the RTC timestamp is still in non-DST time
				{
					// If we are prior or equal to DST fallback instant, the clock needs to be forwarded 1 h
					// as we are still on DST
					if (currentTime -> hour <= 2)
					{
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				if ((6 + currentTime -> dayM - currentTime -> dayW) / 7 >= 1)
				{
					return 0;
				}
				else
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

unsigned char checkDST(volatile time_data* currentTime)
{
	if (!dstData.dstEnabled)
	{
		return 0;
	}
	if (dstData.dstRegion == DST_EU)
	{
		if (dstData.dstStatus == 0)
		{
			if (currentTime -> month == MARZO)
			{
				if (currentTime -> dayW == DOMINGO)
				{
					if ((DAYS_OF_MARCH - currentTime -> dayM) < 7)
					{
						if (currentTime -> hour == dstData.dstForwardHour)
						{
							if (currentTime -> min == 0)
							{
								if (currentTime -> sec == 0)
								{
									currentTime -> hour = currentTime -> hour + 1;
									dstData.dstStatus = 1;
									return 1;
								}
								return 0;
							}
							return 0;
						}
						return 0;
					}
					return 0;
				}
				return 0;
			}
			return 0;
		}
		else
		{
			if (currentTime -> month == OCTUBRE)
			{
				if (currentTime -> dayW == DOMINGO)
				{
					if ((DAYS_OF_OCTOBER - currentTime -> dayM) < 7)
					{
						if (currentTime -> hour == dstData.dstForwardHour + 1) // Todo think how to optimize this addition
						{
							if (currentTime -> min == 0)
							{
								if (currentTime -> sec == 0)
								{
									currentTime -> hour = currentTime -> hour - 1;
									dstData.dstStatus = 0;
									return 1;
								}
								return 0;
							}
							return 0;
						}
						return 0;
					}
					return 0;
				}
				return 0;
			}
			return 0;
		}		
	}
	if (dstData.dstRegion == DST_USA)
	{
		if (dstData.dstStatus == 0)
		{
			if (currentTime -> month == MARZO)
			{
				if (currentTime -> dayW == DOMINGO)
				{
					if ((currentTime -> dayM >= 8) && (currentTime -> dayM <= 14))
					{
						if (currentTime -> hour == 2)
						{
							if (currentTime -> min == 0)
							{
								if (currentTime -> sec == 0)
								{
									currentTime -> hour = currentTime -> hour + 1;
									dstData.dstStatus = 1;
									return 1;
								}
								return 0;
							}
							return 0;
						}
						return 0;
					}
					return 0;
				}
				return 0;
			}
			return 0;
		}
		else
		{
			if (currentTime -> month == NOVIEMBRE)
			{
				if (currentTime -> dayW == DOMINGO)
				{
					if (currentTime -> dayM <= 7)
					{
						if (currentTime -> hour == 2)
						{
							if (currentTime -> min == 0)
							{
								if (currentTime -> sec == 0)
								{
									currentTime -> hour = currentTime -> hour - 1;
									dstData.dstStatus = 0;
									return 1;
								}
								return 0;
							}
							return 0;
						}
						return 0;
					}
					return 0;
				}
				return 0;
			}
			return 0;
		}
	}
	return 0;
}

unsigned char updateDSTOnStartup(volatile time_data* currentTime)
{
	if (!dstData.dstEnabled)
	{
		return 0;
	}
	if (isCurrentTimeDST(currentTime))
	{
		if (dstData.dstStatus) // Last time clock was on was during DST time and we still are on DST
		{
			// Do nothing
			return 0;
		}
		else // Last time clock was on was during non-DST time, so it needs to forward 1 h and set DST flag		
		{
			(currentTime -> hour) = (currentTime -> hour) + 1;
			dstData.dstStatus = 1;
			return 1;
		}
	}
	else
	{
		if (dstData.dstStatus) // Last time clock was on was during DST time but we are not on DST now, so it needs to fall 1 h back
		{
			(currentTime -> hour) = (currentTime -> hour) - 1;
			dstData.dstStatus = 0;
			return 1;
		}
		else // Last time clock was on was during non-DST time and we are not in DST now
		{
			// Do nothing
			return 0;
		}
	}
}

unsigned char persistDSTData(void)
{
	uint8_t raw;
	raw = (((((uint8_t) dstData.dstRegion) & 1) << 7) |
		  ((dstData.dstEnabled & 1) << 6) |
		  ((dstData.dstStatus & 1) << 5) |
	      (dstData.dstForwardHour & 0b00011111));    
	eeprom_busy_wait();
	eeprom_update_byte((uint8_t*) DST_EEPROM_ADDRESS, raw);
	return 1;
}
