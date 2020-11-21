

#include "ht16k33_common_anode_display.h"

const unsigned char seven_segment_excitation_sequence[16]={0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
const unsigned char fourteen_segment_excitation_sequence[16]={0, 3, 5, 1, 4, 2, 13, 9, 7, 11, 6, 8, 12, 10, 15, 14}; //Segments excitated by groups of two
//const unsigned char fourteen_segment_excitation_sequence[16]={0, 5, 4, 13, 7, 6, 12, 15, 3, 1, 2, 9, 11, 8, 10, 14}; //Segments excitated by groups of two

// Arranges data to be sent to the display
// Receives the own transmission buffer, its size (number of segments to update according to excitation sequence, if it is lower than 15, it doesn't update decimal dot flags),
// the segments-to_digits mapping array pointer (which are the segments controlled by the byte that is sent at the same position as it is in the array),
// the pointer to the function that draws the digit for that display and the string input buffer.
//Returns the own transmission buffer
char* data_arrange (char* segment_output_buffer, char segments_number, const unsigned char* excitation_sequence,  unsigned int (*characterMappingFunction) (unsigned char), unsigned char* character_buffer)
{
	unsigned char segment_index, input_index, segment_mask_index; //Iterators
	unsigned int segments_array[CHARACTER_BUFFER_SIZE]; //Casted-to-14-segments character buffer
	
	for (segment_index=0; segment_index<segments_number; ++segment_index) //For each segment
	{
		segment_output_buffer[segment_index]=0; //Clear its buffer value
		segment_mask_index=excitation_sequence[segment_index]; //Set where is placed the information of the segment being processed in the input buffer
		for(input_index=0; input_index<CHARACTER_BUFFER_SIZE && input_index<8*sizeof(segment_output_buffer[segment_index]); ++input_index)	//For all the characters present in the input buffer
		{
			if (segment_index==0) //If it is the first iteration over the character buffer (first segment to be setted)
			{

				segments_array[input_index]=(*characterMappingFunction)(character_buffer[input_index]); //Character buffer is previously casted into segments mask information
			}
			if (segment_mask_index>input_index) //If the current evaluated segment is more to the left of the current character being processed
			{
				segment_output_buffer[segment_index]|= (unsigned char) ((segments_array[input_index] & (1<< segment_mask_index)) >> (segment_mask_index-input_index)); // Current segment bit is masked and shifted to the right
			}
			else
			{
				segment_output_buffer[segment_index]|= (unsigned char) ((segments_array[input_index] & (1<< segment_mask_index)) << (input_index-segment_mask_index)); // Else is shifted to the left			}
			}
		}
	}
	return segment_output_buffer;
}

char* data_arrange_7segments (char* segment_output_buffer, char segments_number, unsigned char* character_buffer)
{
	return data_arrange(segment_output_buffer, segments_number, seven_segment_excitation_sequence, ascii_2_7segment, character_buffer);
}


char* data_arrange_14segments (char* segment_output_buffer, char segments_number, unsigned char* character_buffer)
{
	return data_arrange(segment_output_buffer, segments_number, fourteen_segment_excitation_sequence, ascii_2_14segment, character_buffer);
}

//Sets decimal dot in a defined position
//Receives the own display buffer before being sent, the dot slot position on it and the digit where the dot has to be set
//Returns the display buffer
char* set_decimal_dot (char* data_buffer, unsigned char dot_slot_position, char dot_position)
{
	data_buffer[dot_slot_position]|= (1<<dot_position);
	return data_buffer;
}

//Clears decimal dot in a defined position
//Receives the own display buffer before being sent, the dot slot position on it and the digit where the dot has to be clear
//Returns the display buffer
char* clear_decimal_dot (char* data_buffer, unsigned char dot_slot_position, char dot_position)
{
	data_buffer[dot_slot_position]&= ~(1<<dot_position);
	return data_buffer;
}

//Sets multiple decimal dots with a mask
//Receives the own display buffer before being sent, the dot slot position on it and the 8 digit dot slot mask to be set
//It doesn't override decimal dots previously setted
//Returns the display buffer
char* set_multiple_decimal_dot (char* data_buffer, unsigned char dot_slot_position, unsigned char dots_mask)
{
	data_buffer[dot_slot_position]|= dots_mask;
	return data_buffer;
}

//Sets or clears decimal dots slot
//Receives the own display buffer before being sent, the dot slot position and the decimal dots mask to be repalced with
//It overrides decimal dots previously setted
//Returns the display buffer
char* override_all_decimal_dots (char* data_buffer, unsigned char dot_slot_position, unsigned char dots_mask)
{
	data_buffer[dot_slot_position]&= dots_mask;
	return data_buffer;
}



//Casts between ASCII char value to 7 segments display mask	
unsigned int ascii_2_7segment(unsigned char character)
{
    switch (character)
    {
        case '0': return 0b00111111;
        case '1': return 0b00000110;
        case '2': return 0b01011011;
        case '3': return 0b01001111;
        case '4': return 0b01100110;
        case '5': return 0b01101101;
        case '6': return 0b01111101;
        case '7': return 0b00000111;
        case '8': return 0b01111111;
        case '9': return 0b01101111;

        case 'a':
        case 'A': return 0b01110111;
        case 'b':
        case 'B': return 0b01111100;
        case 'c': return 0b01011000;
        case 'C': return 0b00111001;
        case 'd':
        case 'D': return 0b01011110;
        case 'e': return 0b01111011;
        case 'E': return 0b01111001;
        case 'f':
        case 'F': return 0b01110001;
        case 'g': return 0b01101111;
        case 'G': return 0b00111101;
        case 'h': return 0b01110100;
        case 'H': return 0b01110110;
        case 'i': return 0b00010000;
        case 'I': return 0b00110000;
        case 'j': return 0b00011110;
        case 'J': return 0b00011111;
        case 'k':
        case 'K': return 0b01110000;
        case 'l': return 0b00110000;
        case 'L': return 0b00111000;
        case 'm': return 0b01010100;
        case 'M': return 0b00110111;
        case 'n': return 0b01010100;
        case 'N': return 0b00110111;
        case 164: //Ã±
        case 165: return 0b01010101;
        case 'o': return 0b01011100;
        case 'O': return 0b00111111;
        case 'p':
        case 'P': return 0b01110011;
        case 'q': return 0b11011100;
        case 'Q': return 0b10111111;
        case 'r': return 0b01010000;
        case 'R': return 0b00110001;
        case 's':
        case 'S': return 0b01101101;
        case 't': return 0b01111000;
        case 'T': return 0b00110001;
        case 'u': return 0b00011100;
        case 'U': return 0b00111110;
        case 'v': return 0b00011100;
        case 'V': return 0b00111110;
        case 'w': return 0b10011100;
        case 'W': return 0b10111110;
        case 'x':
        case 'X': return 0b01110110;
        case 'y': return 0b01101110;
        case 'Y': return 0b01101110;
        case 'z':
        case 'Z': return 0b01101101;

        case ' ': return 0;
        case '-': return 0b01000000;
        case '!': return 0b10000010;
        case '.':
        case ',': return 0b10000000;

        case '_':
        default: return 0b00001000; //_

    }
}

//Casts between ASCII char value to 14 segments display mask
unsigned int ascii_2_14segment(unsigned char character)
{
    switch (character)
    {
        case '0': return 0b0000000000111111;
        case '1': return 0b0000000100000110;
        case '2': return 0b0010001000011011;
        case '3': return 0b0000001000001111;
        case '4': return 0b0010001000100110;
        case '5': return 0b0010001000101101;
        case '6': return 0b0010001000111101;
        case '7': return 0b0000000000000111;
        case '8': return 0b0010001000111111;
        case '9': return 0b0010001000101111;

        case 'a': return 0b0010001000011111;
        case 'A': return 0b0010001000110111;
        case 'b': return 0b0010001000111100;
        case 'B': return 0b0000101010001111;
        case 'c': return 0b0010001000011000;
        case 'C': return 0b0000000000111001;
        case 'd': return 0b0010001000001110;
        case 'D': return 0b0000100010001111;
        case 'e': return 0b0010001000111011;
        case 'E': return 0b0010000000111001;
        case 'f': return 0b0010000000110001;
        case 'F': return 0b0010001000110001;
        case 'g': return 0b0010011000100111;
        case 'G': return 0b0000001000111101;
        case 'h': return 0b0010001000110100;
        case 'H': return 0b0010001000110110;
        case 'i': return 0b0000100010000000;
        case 'I': return 0b0000100010001001;
        case 'j': return 0b0000010000000110;
        case 'J': return 0b0000000000011111;
        case 'k': return 0b0000110110000000;
        case 'K': return 0b0010010100110000;
        case 'l': return 0b0000100010000000;
        case 'L': return 0b0000000000111000;
        case 'm': return 0b0010101000010100;
        case 'M': return 0b0000000101110110;
        case 'n': return 0b0010001000010100;
        case 'N': return 0b0000010001110110;
        case 164: return 0b0010001000010100; // ñ
        case 165: return 0b0000010001110111; // Ñ
        case 'o': return 0b0010001000011100;
        case 'O': return 0b0000000000111111;
        case 'p': return 0b0000101010000011;
        case 'P': return 0b0010001000110011;
        case 'q': return 0b0010001000100111;
        case 'Q': return 0b0000010000111111;
        case 'r': return 0b0010000000010000;
        case 'R': return 0b0010011000110011;
        case 's': return 0b0000001001001101;
        case 'S': return 0b0010001000101101;
        case 't': return 0b0010000000111000;
        case 'T': return 0b0000100010000001;
        case 'u': return 0b0000000000011100;
        case 'U': return 0b0000000000111110;
        case 'v': return 0b0001000000010000;
        case 'V': return 0b0001000100110000;
        case 'w': return 0b0001010000010100;
        case 'W': return 0b0001010000110110;
        case 'x':
        case 'X': return 0b0001010101000000;
        case 'y': return 0b0000001010001110;
        case 'Y': return 0b0000100101000000;
        case 'z': return 0b0001001100001001;
        case 'Z': return 0b0001000100001001;

        case ' ': return 0;
		case '~':
        case '-': return 0b0010001000000000;
		case '?': return 0b1000101000000011;
        case '!': return 0b1000000000000010;
		case '·':
        case '.': return 0b1000000000000000;
        case ',': return 0b0001000000000000;
		case '(':
		case '[': return 0b0000000000111001;
		case ')':
		case ']': return 0b0000000000001111;
		case '*': return 0b0001110111000000;
		case '+': return 0b0010101010000000;
		case 241: return 0b0010101010001000; //+-
		case '/': return 0b0001000100000000;
		case '\\': return 0b0000010001000000;
		case '|': return 0b0000100010000000;
		case '>': return 0b0001000001000000;
		case '<': return 0b0000010100000000;
		case '=': return 0b0010001000001000;
		case '\'': return 0b0000000010000000;
		case '"': return 0b0000000010000010;
		case '´': return 0b0000000100000000;
		case '`': return 0b0000000001000000;
		case '¬': return 0b0010001000000100;
		case '%': return 0b0011011101100000;
		case '@': return 0b0010100000011111;


        case '_':
        default:  return 0b0000000000001000; //_

    }
}


