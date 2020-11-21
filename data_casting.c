/*
 * data_casting.c
 *
 * Created: 01/11/2017 10:05:03
 *  Author: josefe
 */ 

unsigned char bcd2char(unsigned char bcd)
{
	return ((bcd & 0x0F)+'0');
}

unsigned char bcd2dec(unsigned char bcd)
{
	return ((bcd & 0x0F)+ (((bcd & 0xF0)>>3)*5));
}

unsigned char dec2bcd(unsigned char dec)
{
	return ((((dec/10) <<4) & 0xF0) | ((dec%10) & 0x0F));
}