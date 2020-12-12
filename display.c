
#include "display.h"

#include "ht16k33_common_anode_display.h"
#include "TWI_Master.h"

const unsigned char WHEEL_POSITIONS[8] = {0, 1, 5, 4, 6, 2, 3, 7};

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
	unsigned char aux;
	
	display_TX_buffer_t.fields.TWI_address=I2C_display_address;
	display_TX_buffer_t.fields.command=HT16K33_RAM_BEGIN_ADDRESS; //Points to data input
		
	switch(display_type)		
	{
		case DISPLAY_5642BG:
			data_arrange_7segments(display_TX_buffer_t.fields.segments, HT16K33_SEGMENTS_BUFFER_SIZE, data_string);
			set_multiple_decimal_dot (display_TX_buffer_t.fields.segments, SPECIAL_DOTS_SLOT_56INCH, special_dots_mask);
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
		case DISPLAY_DVD:
			// Remap the string to comply with this display layout
			 aux = data_string[0]; // Backup zero character
			data_string[0] = data_string[2]; // Second character must be sent at the zero position
			data_string[2] = data_string[1]; // First character must be sent at the second position
			data_string[1] = aux; // Zero character must be sent at the first position
			data_string[7] = ' '; // Only 7 characters, 8th must be off to not override the last dots
			data_arrange_7segments(display_TX_buffer_t.fields.segments, HT16K33_SEGMENTS_BUFFER_SIZE, data_string);
			remap_dvd_display(display_TX_buffer_t.fields.segments);
			// Set wheel dots
			for (unsigned char i = 0; i < 7 ; ++i) // For each chunk of two bytes that hold all the information for a single segment along the display
			{
				// Check if the cog whose value is sent at this position at the transmission buffer has to be set
				// (we're iterating along the frame, so the cog mask reading order is set by the byte who controls it at the display and therefore is random).
				if (special_dots_mask & (1 << WHEEL_POSITIONS[i]))
				{
					set_decimal_dot(display_TX_buffer_t.fields.segments, 2 * i, 1);
				}
				else
				{
					clear_decimal_dot(display_TX_buffer_t.fields.segments, 2 * i, 1);
				}
			}
			if (special_dots_mask & (1 << WHEEL_POSITIONS[7])) // 8th cog goes at the same frame that 7th but one bit before
			{
				set_decimal_dot(display_TX_buffer_t.fields.segments, 2 * 6, 0);
			}
			else
			{
				clear_decimal_dot(display_TX_buffer_t.fields.segments, 2 * 6, 0);
			}
			// Decimal dot
			if (decimal_dots_mask & 1) // Hour-minute colons
			{
				set_decimal_dot(display_TX_buffer_t.fields.segments, 3, 1);
			}

			else
			{
				clear_decimal_dot(display_TX_buffer_t.fields.segments, 3, 1);
			}

			if (decimal_dots_mask & 2) // Minute-seconds colons
			{
				set_decimal_dot(display_TX_buffer_t.fields.segments, 5, 1);
			}
			else
			{
				clear_decimal_dot(display_TX_buffer_t.fields.segments, 5, 1);
			}
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


// Clears the string display
void clear_buffer(unsigned char* data_string, unsigned char length)
{
	for (unsigned char i = 0; i < 8; ++i)
	{
		data_string[i] = ' ';
	}
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
