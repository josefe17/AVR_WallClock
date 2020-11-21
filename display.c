
#include "display.h"

#include "ht16k33_common_anode_display.h"
#include "TWI_Master.h"

//Union for data transmission
union display_TX_buffer
{
	unsigned char array[HT16K33_TWI_BUFFER_SIZE];
	struct
	{
		unsigned char TWI_address;
		unsigned char command;
		unsigned char segments[HT16K33_SEGMENTS_BUFFER_SIZE];
	}fields;
}display_TX_buffer_t;

//Updates data on the screen
//Receives I2C address, the string to be formatted and sent, and the extra dots information
void display_update(unsigned char I2C_display_address, unsigned char display_type, unsigned char* data_string, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	//unsigned char aux;		
	
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_RAM_BEGIN_ADDRESS; //Points to data input
		
	/*
	for (aux=0; aux<HT16K33_SEGMENTS_BUFFER_SIZE; ++aux)
	{
		display_TX_buffer_t.fields.segments[aux]=0;
	}
	*/	
	switch(display_type)		
	{
		case DISPLAY_5642BG:
			data_arrange_7segments(display_TX_buffer_t.fields.segments, HT16K33_SEGMENTS_BUFFER_SIZE, data_string);
			set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT_56INCH, special_dots_mask);
			//set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT, special_dots_mask);
		break;
		case DISPLAY_3942BG:
		default:
			data_arrange_7segments(display_TX_buffer_t.fields.segments, HT16K33_SEGMENTS_BUFFER_SIZE, data_string);
			set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, DECIMAL_DOTS_SLOT, decimal_dots_mask);
			set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT, special_dots_mask);
		break;
		case DISPLAY_PDA54_14SEGMENTS:
			data_arrange_14segments(display_TX_buffer_t.fields.segments, HT16K33_SEGMENTS_BUFFER_SIZE, data_string);
		break;
	}	 	

			
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_TWI_BUFFER_SIZE);
}

//Updates only the dots information on the screen
void dots_update(unsigned char I2C_display_address,unsigned char display_type, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_RAM_BEGIN_ADDRESS;
	
	switch(display_type)
	{
		case DISPLAY_5642BG:
		set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT_56INCH, special_dots_mask);
		//set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT, special_dots_mask);
		break;
		
		case DISPLAY_3942BG:
		default:
		set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, DECIMAL_DOTS_SLOT, decimal_dots_mask);
		set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT, special_dots_mask);
		break;
	}
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_TWI_BUFFER_SIZE);
}

//Clears both display and TX buffer
void clear_display(unsigned char I2C_display_address)
{
	unsigned char aux;		
	
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_RAM_BEGIN_ADDRESS; //Points to data input
			
	for (aux=0; aux<HT16K33_SEGMENTS_BUFFER_SIZE; ++aux)
	{
		display_TX_buffer_t.fields.segments[aux]=0;
	}
	
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_TWI_BUFFER_SIZE);
}

//Initialises display
void display_init(unsigned char I2C_display_address)
{
	//Init oscillator
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_OSCILLATOR_ON;
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_COMMAND_BUFFER_SIZE);
	//Turn on display
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_DISPLAY_ON;
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_COMMAND_BUFFER_SIZE);
}

//Turns display on and updates brightness
void turn_on_and_blink_display(unsigned char I2C_display_address, unsigned char display_blinking_mode)
{
	if ((display_blinking_mode!= HT16K33_DISPLAY_BLINKING_2HZ) && (display_blinking_mode!= HT16K33_DISPLAY_BLINKING_1HZ) && (display_blinking_mode!= HT16K33_DISPLAY_BLINKING_05HZ))
	{
		display_blinking_mode=HT16K33_DISPLAY_BLINKING_OFF; //If it is set a non valid blinking value, blinking is disabled
	}
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=( HT16K33_DISPLAY_ON | display_blinking_mode);
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_COMMAND_BUFFER_SIZE);
}

//Turns display off
void turn_off_display(unsigned char I2C_display_address)
{
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_DISPLAY_OFF;
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_COMMAND_BUFFER_SIZE);
}

//Sets brightness value
void set_brightness_display(unsigned char I2C_display_address, unsigned char brightness)
{
	//Brightness value anti overflow
	if (brightness>=HT16K33_DISPLAY_DIMMER_MASK)
	{
		brightness=HT16K33_DISPLAY_DIMMER_MASK;
	}
	if (brightness<0)
	{
		brightness=0;
	}
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=( HT16K33_DISPLAY_DIMMER_ADDRESS | brightness );
	TWI_Start_Transceiver_With_Data(display_TX_buffer_t.array, HT16K33_COMMAND_BUFFER_SIZE);
}
