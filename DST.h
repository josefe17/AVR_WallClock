/*
 * DST.h
 *
 * Created: 04/10/2023 13:01:04
 *  Author: josefe
 */ 


#ifndef DST_H_
#define DST_H_

#include "date.h"

typedef enum
{
	DST_EU,
	DST_USA	
}DSTRegions;

typedef struct
{
	DSTRegions dstRegion: 1;
	unsigned char dstEnabled: 1;
	unsigned char dstStatus: 1;
	unsigned char dstForwardHour: 5;
}DSTData;

void DSTInit(void);

void enableDST(void);
void disableDST(void);
unsigned char getDSTEnabledStatus(void);

void setDSTEURegion(void);
void setDSTUSARegion(void);
DSTRegions getDSTRegion(void);

void setEUDSTForwardHour (unsigned char hour); // Return hour is forward + 1, as both happens at GMT + 1
unsigned char getEUDSTForwardHour(void);

unsigned char checkDST(time_data* currentTime);

unsigned char updateDSTOnStartup(time_data* currentTime);

// Todo printf functions for enabled, regions and forward time





#endif /* DST_H_ */