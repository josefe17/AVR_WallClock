/*
 * DST.c
 *
 * Created: 04/10/2023 13:21:11
 *  Author: josefe
 */ 

#include "DST.h"

DSTData dstData;

unsigned char isCurrentTimeDST(time_data currentTime);

void DSTInit(void);

void enableDST(void)
{
	dstData.dstEnabled = 1;	
}

void disableDST(void)
{
	dstData.dstEnabled = 0;
}

unsigned char getDSTEnabledStatus(void)
{
	return dstData.dstEnabled;
}

void setDSTEURegion(void)
{
	dstData.dstRegion = DST_EU;	
}

void setDSTUSARegion(void)
{
	dstData.dstRegion = DST_USA;
}

DSTRegions getDSTRegion(void)
{
	return dstData.dstRegion;
}

void setEUDSTForwardHour (unsigned char hour)
{
	dstData.dstForwardHour = hour & 0b00011111;
}

unsigned char getEUDSTForwardHour(void)
{
	return dstData.dstForwardHour;
}

unsigned char checkDST(time_data* currentTime)
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
					if ((DAYS_OF_MARCH - currentTime -> dayM >= 8) && (DAYS_OF_MARCH - currentTime -> dayM <= 14))
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

unsigned char updateDSTOnStartup(time_data* currentTime)
{
	
}

unsigned char isCurrentTimeDST(time_data currentTime)
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
		
		if (currentTime -> month == MARZO)
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
				// It is impossible to know if DST rewind hour (e.g. 2 to 3 AM of the last sunday of october) is DST or not by itself 
				// because it can be both, so I always considered it DST for both natural and 'repeated' hour.
				// If you turn the clock on between 2 am and 3 am the last sunday of october when the clock has already fallen back (second 2 AM hour)
				// and the last time that the clock was on was during non-DST time, it will unavoidably forward 1 h despite time being right because it thinks
				// that we are still on DST.
				if (currentTime -> hour >= dstData.dstForwardHour + 1)
				{
					return 0;
				}
				else
				{
					return 1;
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
		if (dstData.dstStatus == 0)
		{
			if (currentTime -> month == MARZO)
			{
				if (currentTime -> dayW == DOMINGO)
				{
					if ((DAYS_OF_MARCH - currentTime -> dayM >= 8) && (DAYS_OF_MARCH - currentTime -> dayM <= 14))
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