/*
 * lm75b_basic_driver.c
 *
 *  Created on: 12 dic. 2020
 *      Author: josefe
 */

#include "lm75b_basic_driver.h"
#include "TWI_Master.h"

signed int readLM75BTemperature_1deg(unsigned char i2c_address)
{
	unsigned char msg_buff[2];
	msg_buff[0] = i2c_address;
	msg_buff[1] = 0; // Pointer to temperture register
	TWI_Start_Transceiver_With_Data(msg_buff, 2);
	//Retrieve_data
	msg_buff[0]=i2c_address | 1; // read mode
	TWI_Start_Transceiver_With_Data(msg_buff, 3);
	TWI_Get_Data_From_Transceiver(msg_buff, 3);
	// 11 bit signed int with 0.125 ºC accuracy (LEFT JUSTIFIED, MSByte is integer units)
	signed int temperature = (msg_buff[1] << 8) | msg_buff[2];
	return temperature;
}
