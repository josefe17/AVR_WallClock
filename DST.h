/*
 * DST.h
 *
 * Created: 04/10/2023 13:01:04
 *  Author: josefe
 */ 


#ifndef DST_H_
#define DST_H_

#include "date.h"

#define DST_EEPROM_ADDRESS 10

typedef enum
{
	DST_EU = 0,
	DST_USA = 1	
}DSTRegions;

typedef struct
{
	DSTRegions dstRegion: 1;
	unsigned char dstEnabled: 1;
	unsigned char dstStatus: 1;
	unsigned char dstForwardHour: 5;
}DSTData;

unsigned char DSTInit(volatile time_data* currentTime);
unsigned char processDST(volatile time_data* currentTime);
unsigned char getDSTCurrentStatus(void);

unsigned char enableDST(void);
unsigned char disableDST(void);
unsigned char getDSTEnabledStatus(void);
char* getDSTNameString(void);
char* getDSTOnString(void);
char* getDSTOffString(void);

unsigned char setDSTEURegion(void);
unsigned char setDSTUSARegion(void);
DSTRegions getDSTRegion(void);
char* getDSTRegionCaptionString(void);
char* getDSTEURegionString(void);
char* getDSTUSARegionString(void);

unsigned char setEUDSTForwardHour (unsigned char hour); // Return hour is forward + 1, as both happens at GMT + 1
unsigned char getEUDSTForwardHour(void);
char* getDSTForwardHourCaptionString(void);


#endif /* DST_H_ */