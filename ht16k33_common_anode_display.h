/*
 * ht16k33_common_anode_display.h
 *
 * Created: 22/07/2017 18:25
 *  Author: josefe
 */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HT16K33_COMMON_ANODE_DISPLAY_H_
#define HT16K33_COMMON_ANODE_DISPLAY_H_

// Number of characters. Limited by COM outputs
#define CHARACTER_BUFFER_SIZE 8

char* data_arrange (char* segment_output_buffer, char segments_number, const unsigned char* excitation_sequence,  unsigned int (*characterMappingFunction) (unsigned char), unsigned char* character_buffer);
char* data_arrange_7segments (char* segment_output_buffer, char segments_number, unsigned char* character_buffer);
char* data_arrange_14segments (char* segment_output_buffer, char segments_number, unsigned char* character_buffer);
char* set_decimal_dot (char* data_buffer, unsigned char dot_slot_position, char dot_position);
char* clear_decimal_dot (char* data_buffer, unsigned char dot_slot_position, char dot_position);
char* set_multiple_decimal_dot (char* data_buffer, unsigned char dot_slot_position, unsigned char dots_mask);
char* override_all_decimal_dots (char* data_buffer, unsigned char dot_slot_position, unsigned char dots_mask);
unsigned int ascii_2_7segment(unsigned char character);
unsigned int ascii_2_14segment(unsigned char character);


#endif 

#ifdef __cplusplus
}
#endif
