/*
 * display.h
 *
 * Created: 21/09/2017 15:04
 *  Author: josefe
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DISPLAY_H_
#define DISPLAY_H_

//TX buffer sizes
#define HT16K33_TWI_BUFFER_SIZE 		18
#define HT16K33_COMMAND_BUFFER_SIZE		2
#define HT16K33_SEGMENTS_BUFFER_SIZE 	16

//Commands
#define HT16K33_RAM_BEGIN_ADDRESS		0

#define HT16K33_OSCILLATOR_ON			0b00100001
#define HT16K33_OSCILLATOR_OFF			0b00100000

#define HT16K33_DISPLAY_ON				0b10000001	
#define HT16K33_DISPLAY_OFF				0b10000000

#define HT16K33_DISPLAY_BLINKING_OFF	0b10000000
#define HT16K33_DISPLAY_BLINKING_2HZ	0b10000010
#define HT16K33_DISPLAY_BLINKING_1HZ	0b10000100
#define HT16K33_DISPLAY_BLINKING_05HZ	0b10000110

#define HT16K33_DISPLAY_DIMMER_ADDRESS	0b11100000
#define HT16K33_DISPLAY_DIMMER_MASK		0b00001111

//Dots masks
#define DECIMAL_DOTS_SLOT 				14
#define SPECIAL_DOTS_SLOT 				15
#define SPECIAL_DOTS_SLOT_56INCH		14
#define FIRST_COLON_MASK 				1
#define	FIRST_DEGREES_DOT_MASK  		2
#define SECOND_COLON_MASK 				4
#define	SECOND_DEGREES_DOT_MASK 		8
#define FIRST_COLON_56INCH_MASK			2
#define FIRST_COLON_56INCH_SHIFTS		1


//Display_version
#define DISPLAY_3942BG					0
#define DISPLAY_5642BG					1
#define DISPLAY_PDA54_14SEGMENTS		2

//Prototypes
void display_update(unsigned char I2C_display_address, unsigned char display_type, unsigned char* data_string, unsigned char decimal_dots_mask, unsigned char special_dots_mask);
void dots_update(unsigned char I2C_display_address, unsigned char display_type, unsigned char decimal_dots_mask, unsigned char special_dots_mask);
void clear_display(unsigned char I2C_display_address);
void display_init(unsigned char I2C_display_address);
void turn_on_and_blink_display(unsigned char I2C_display_address, unsigned char display_blinking_mode);
void turn_off_display(unsigned char I2C_display_address);
void set_brightness_display(unsigned char I2C_display_address, unsigned char brightness);


#endif 

#ifdef __cplusplus
}
#endif
