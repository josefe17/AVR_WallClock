
#define F_CPU 16000000UL

#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "data_casting.h"
#include "display.h"
#include "rtc.h"
#include "TWI_Master.h"
#include "timer0_tick.h"
#include "fastSkipTimer.h"
#include "lm75b_basic_driver.h"
#include "fsm.h"
#include "GPIO.h"

#define MS_TIMER_COUNT	78
#define MS_DELAY_CYCLES	1

#define FAST_SKIP_TIMER_DELAY_CYCLES 1000 //MS_DELAY_CYCLES times  = 1 s

#define THERMISTOR_BETA 3950.0
#define CESIUS2KELVIN_CONSTANT 273.15
#define THERMISTOR_SERIES_RESISTOR_OHMS 10000.0
#define THERMISTOR_NOMINAL_OHMS 10000.0
#define THERMISTOR_TEMPERATURE_NOMINAL (CESIUS2KELVIN_CONSTANT+25.0)


/*HW ADDRESSES*/
#define HT16K33_1_WRITE_ADDRESS	0b11100000
#define HT16K33_2_WRITE_ADDRESS	0b11100010
#define HT16K33_3_WRITE_ADDRESS	0b11100100
#define HT16K33_4_WRITE_ADDRESS	0b11100110
#define HT16K33_5_WRITE_ADDRESS	0b11101000
#define DS3231_WRITE_ADDRESS	0b11010000
#define LM75B_WRITE_ADDRESS 	0b10010000
#define BRIGHTNESS_CHANNEL		3

/*Data buffers*/
unsigned char str_buffer[8] = "        ";
unsigned char display1_buffer[8] = "        ";
unsigned char display1_decimal_dots_buffer = 0;
unsigned char display1_special_dots_buffer = 0;
unsigned char display2_buffer[8] = "        ";
unsigned char display2_decimal_dots_buffer = 0;
unsigned char display2_special_dots_buffer = 0;
unsigned char display3_buffer[8] = "        ";
unsigned char display3_decimal_dots_buffer = 0;
unsigned char display3_special_dots_buffer = 0;
unsigned char display4_buffer[8] = "        ";
unsigned char display4_decimal_dots_buffer = 0;
unsigned char display4_special_dots_buffer = 0;
const unsigned char str_buffer_test[11]="0123456    ";
volatile time_data current_time;
unsigned char power_fail_flag;

/*Cogwheel resources*/
unsigned char lastSecondAtCogwheelUpdate;
unsigned char cogwheelLastMask;

/* Temperature resources */
signed int ambientTemperature;
double thermistorTemperature;

/* ADC resources */
volatile unsigned char adc_buffer;

/*Prototypes*/
/*ADC*/
void InitADC(void);
unsigned char ReadADC(unsigned char ADCchannel);

/*Clock FSM input functions*/
unsigned char check_no_buttons(fsm_t* this);
unsigned char check_skip_long_press(fsm_t* this);
unsigned char check_any_buttons(fsm_t* this);
unsigned char check_idle(fsm_t* this);
unsigned char check_set_clock(fsm_t* this);
unsigned char check_arrows(fsm_t* this);
unsigned char check_blinking_flag(fsm_t* this);
unsigned char check_no_blinking_flag(fsm_t* this);
unsigned char check_fast_skip_timer(fsm_t* this);

/*Clock FSM output functions*/
void showtime(fsm_t* this);
void showtime_and_stop_timer(fsm_t* this);
void showYear (fsm_t* this);
void showYear_and_stop_timer(fsm_t* this);
void updatetime(fsm_t* this);
void blinktime(fsm_t* this);
void clear_blinktime_and_showtime(fsm_t* this);
void clear_oscillator_fault_and_showtime(fsm_t* this);
void blinkhour(fsm_t* this);
void modify_hour(fsm_t* this);
void modify_hour_once(fsm_t* this);
void blinkminute(fsm_t* this);
void modify_minute(fsm_t* this);
void modify_minute_once(fsm_t* this);
void blinkdayM (fsm_t* this);
void modify_dayM(fsm_t* this);
void modify_dayM_once(fsm_t* this);
void blinkMonth (fsm_t* this);
void modify_month(fsm_t* this);
void modify_month_once(fsm_t* this);
void blinkYear (fsm_t* this);
void modify_year(fsm_t* this);
void modify_year_once(fsm_t* this);

/* Cogwheel functions*/
void startCogWheel();
unsigned char updateCogwheel();

/* Thermometer functions */
void startThermometer();
char* readThermometer();
char* readThermometerDecimal();

/* Thermistor functions */
char* printThermistorTemperature();
char* printThermistorTemperatureDecimal();
double readThermistor (unsigned char rawReading);

/* Display_functions */
unsigned char updateDisplay1(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask);
unsigned char updateDisplay2(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask);
unsigned char updateDisplay3(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask);
unsigned char updateDisplay4(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask);

/*Clock FSM states*/
typedef enum
{
	NULL_STATE,
	IDLE,
	DEBOUNCE_IDLE,
	BLINKING,
	DEBOUNCE_BLINKING,	
	SETHOUR,
	SETHOUR_FAST,
	DEBOUNCE_SETHOUR,
	SETMIN,
	SETMIN_FAST,
	DEBOUNCE_SETMIN,
	SETDAYM,
	SETDAYM_FAST,	
	DEBOUNCE_SETDAYM,
	SETMONTH,
	SETMONTH_FAST,
	DEBOUNCE_SETMONTH,
	SETYEAR,
	SETYEAR_FAST,
	DEBOUNCE_SETYEAR
	}clock_states;

int main (void)
{	
	volatile fsm_t clock_fsm; //Clock FSM model
	
	fsm_trans_t clock_tt[] = {
	{ DEBOUNCE_IDLE,	check_no_buttons,		IDLE,				showtime	},	//If buttons are released got next state
	{ DEBOUNCE_IDLE,	check_any_buttons,		DEBOUNCE_IDLE,		showtime	},	// else wait for release in debounce state
	{ IDLE,				check_blinking_flag,	BLINKING, 			blinktime	},	// If clock fail, display starts blinking
	{ IDLE,				check_set_clock,		DEBOUNCE_SETHOUR, 	showtime	},	// If clock and set are pressed, goes to clock adjustment		
	{ IDLE,				check_idle,				IDLE, 				showtime	},  // just updates display
	
	{ BLINKING,			check_set_clock,		DEBOUNCE_SETHOUR, 	clear_oscillator_fault_and_showtime	},	//If while blinking, clock and set are pressed, turn off global blinking adnd goes to clock adjustment (debounce hour)
	{ BLINKING,			check_blinking_flag,	BLINKING, 			blinktime							},	//Else keep blinking
	{ BLINKING,			check_no_blinking_flag,	IDLE, 				clear_blinktime_and_showtime		},	//if flag is HW cleared, goes back to normal mode		
	
	{DEBOUNCE_SETHOUR,	check_no_buttons,		SETHOUR,			blinkhour	},				//If we are on debounce mode and buttons are released, goes to single adjustment and forces hour to blink
	{DEBOUNCE_SETHOUR,	check_skip_long_press,	SETHOUR_FAST,		showtime_and_stop_timer	},	//If we are on debounce mode and long press is being produced (timeout and arrows long press), goes to fast skip
	{DEBOUNCE_SETHOUR,	check_any_buttons,		DEBOUNCE_SETHOUR,	showtime	},				//If we are on debounce mode and buttons are still pressed and no timeout has been produced, keeps waiting		
	{SETHOUR_FAST,		check_arrows,			SETHOUR_FAST,		modify_hour	},				//If we are on fast skip and arrows are hold down, hour is increased or decreased rapidly (once or each fsm cycle)
	{SETHOUR_FAST,		check_no_buttons,		SETHOUR,			blinkhour	},				//If we are on fast skip and no keys are pressed, goes to single adjustment and forces hour to blink 
	{ SETHOUR,			check_no_buttons,		SETHOUR, 			blinkhour	},				//If we are on single adjustment and no buttons are pressed, hour keeps blinking
	{ SETHOUR,			check_set_clock,		DEBOUNCE_SETMIN, 	blinkminute	},				//If we are on single adjustment and set is pressed, the minutes start blinking and can be modified now
	{ SETHOUR,			check_arrows,			DEBOUNCE_SETHOUR, 	modify_hour_once},			//If we press any of both arrows, clock will increment or decrement once and will go to debounce state waiting for release or long press (also starts fast skip timer)
		
	{DEBOUNCE_SETMIN,	check_no_buttons,		SETMIN,				blinkminute	},				//Minute adjustment, identical as hour
	{DEBOUNCE_SETMIN,	check_skip_long_press,	SETMIN_FAST,		showtime_and_stop_timer	},
	{DEBOUNCE_SETMIN,	check_any_buttons,		DEBOUNCE_SETMIN,	showtime	},	
	{SETMIN_FAST,		check_arrows,			SETMIN_FAST,		modify_minute	},
	{SETMIN_FAST,		check_no_buttons,		SETMIN,				blinkminute	},		
	{ SETMIN,			check_no_buttons,		SETMIN, 			blinkminute	},
	{ SETMIN,			check_set_clock,		DEBOUNCE_SETDAYM,	blinkdayM	},
	{ SETMIN,			check_arrows,			DEBOUNCE_SETMIN,	modify_minute_once	},	
		
	{DEBOUNCE_SETDAYM,	check_no_buttons,		SETDAYM,			blinkdayM	},				//Day adjustment, identical as hour
	{DEBOUNCE_SETDAYM,	check_skip_long_press,	SETDAYM_FAST,		showtime_and_stop_timer	},	
	{DEBOUNCE_SETDAYM,	check_any_buttons,		DEBOUNCE_SETDAYM,	showtime	},
	{SETDAYM_FAST,		check_arrows,			SETDAYM_FAST,		modify_dayM	},
	{SETDAYM_FAST,		check_no_buttons,		SETDAYM,			blinkdayM	},	
	{ SETDAYM,			check_no_buttons,		SETDAYM, 			blinkdayM	},
	{ SETDAYM,			check_set_clock,		DEBOUNCE_SETMONTH,	blinkMonth	},
	{ SETDAYM,			check_arrows,			DEBOUNCE_SETDAYM,	modify_dayM_once	},
		
	{DEBOUNCE_SETMONTH,	check_no_buttons,		SETMONTH,			blinkMonth	},				//Month adjustment, identical as hour
	{DEBOUNCE_SETMONTH,	check_skip_long_press,	SETMONTH_FAST,		showtime_and_stop_timer	},
	{DEBOUNCE_SETMONTH,	check_any_buttons,		DEBOUNCE_SETMONTH,	showtime	},
	{SETMONTH_FAST,		check_arrows,			SETMONTH_FAST,		modify_month},
	{SETMONTH_FAST,		check_no_buttons,		SETMONTH,			blinkMonth	},
	{ SETMONTH,			check_no_buttons,		SETMONTH, 			blinkMonth	},
	{ SETMONTH,			check_set_clock,	    DEBOUNCE_SETYEAR,	blinkYear	},
	{ SETMONTH,			check_arrows,			DEBOUNCE_SETMONTH,	modify_month_once},
			
	{DEBOUNCE_SETYEAR,	check_no_buttons,		SETYEAR,			blinkYear	},				//Year adjustment, changes default blink and show screens
	{DEBOUNCE_SETYEAR,	check_skip_long_press,	SETYEAR_FAST,		showYear_and_stop_timer	},
	{DEBOUNCE_SETYEAR,	check_any_buttons,		DEBOUNCE_SETYEAR,	showYear	},
	{SETYEAR_FAST,		check_arrows,			SETYEAR_FAST,		modify_year	},
	{SETYEAR_FAST,		check_no_buttons,		SETYEAR,			blinkYear	},
	{ SETYEAR,			check_no_buttons,		SETYEAR, 			blinkYear	},
	{ SETYEAR,			check_set_clock,	    DEBOUNCE_IDLE,		updatetime	},
	{ SETYEAR,			check_arrows,			DEBOUNCE_SETYEAR,	modify_year_once},
		
			
	{ -1,			NULL_STATE,				-1,			NULL_STATE		}
	};
	
	
	fsm_init(&clock_fsm, IDLE, clock_tt, 0); //starts FSM
	
	gpio_init();	//Inits GPIO												
	TWI_Master_Initialise(); //Inits I2C
	adc_buffer = 0;
	InitADC();	
	sei();
	display_init(HT16K33_1_WRITE_ADDRESS);
	display_init(HT16K33_5_WRITE_ADDRESS);
	display_init(HT16K33_3_WRITE_ADDRESS);
	display_init(HT16K33_4_WRITE_ADDRESS);
	display_update(HT16K33_1_WRITE_ADDRESS, DISPLAY_3942BG, display1_buffer, display1_decimal_dots_buffer, display1_special_dots_buffer);
	display_update(HT16K33_5_WRITE_ADDRESS, DISPLAY_SC15, display2_buffer, display2_decimal_dots_buffer, display2_special_dots_buffer);
	display_update(HT16K33_3_WRITE_ADDRESS, DISPLAY_PDA54_14SEGMENTS, display3_buffer, display3_decimal_dots_buffer, display3_special_dots_buffer);
	display_update(HT16K33_4_WRITE_ADDRESS, DISPLAY_DVD, display4_buffer, display4_decimal_dots_buffer, display4_special_dots_buffer);
	adc_buffer=(ReadADC(0) >> 4) & 0x0F;
	set_brightness_display(HT16K33_1_WRITE_ADDRESS,adc_buffer);	
	set_brightness_display(HT16K33_5_WRITE_ADDRESS,( adc_buffer + 2 < 16) ? adc_buffer + 2 : adc_buffer);
	set_brightness_display(HT16K33_3_WRITE_ADDRESS,adc_buffer);
	set_brightness_display(HT16K33_4_WRITE_ADDRESS,adc_buffer);
	timer0_tick_init(T0_PRESCALER_256, MS_TIMER_COUNT, MS_DELAY_CYCLES);
	startCogWheel();

	while(1)
	{	
		current_time=retrieve_timestamp_from_RTC(DS3231_WRITE_ADDRESS, DATA_DECIMAL);
		power_fail_flag=check_oscillator_fault(DS3231_WRITE_ADDRESS);
		increment_fast_skip_timer();
		update_button_flags();
		adc_buffer=(ReadADC(0) >> 4) & 0x0F;
		thermistorTemperature=readThermistor(ReadADC(1));
		set_brightness_display(HT16K33_1_WRITE_ADDRESS, adc_buffer);
		set_brightness_display(HT16K33_5_WRITE_ADDRESS, (adc_buffer + 2 < 16) ? adc_buffer + 2 : adc_buffer);
		set_brightness_display(HT16K33_3_WRITE_ADDRESS, adc_buffer);
		set_brightness_display(HT16K33_4_WRITE_ADDRESS, adc_buffer);
		fsm_fire(&clock_fsm);
		delay_until_tick();	
	}	
}

/*FSM input functions*/
unsigned char check_no_buttons(fsm_t* this)
{
	return (button_flags==0);
}

unsigned char check_skip_long_press(fsm_t* this)
{
	return (check_arrows(this) && check_fast_skip_timer(this));
}

unsigned char check_any_buttons(fsm_t* this)
{
	return button_flags;
}

unsigned char check_idle(fsm_t* this)
{
	return ((button_flags & (~SET_BUTTON_MASK))==0); //If no buttons without set are pressed
}

unsigned char check_updown(fsm_t* this)
{
	return ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK)); //If both up and down are pressed simultaneously
}

unsigned char check_no_updown(fsm_t* this)
{
	return (!((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))); //If up or down is released
}

unsigned char check_set_clock(fsm_t* this)
{
	return (button_flags & CLOCK_BUTTON_MASK); //If clock is pressed
}

unsigned char check_arrows(fsm_t* this)
{
	return (button_flags && (UP_BUTTON_MASK | DOWN_BUTTON_MASK));
}

unsigned char check_blinking_flag(fsm_t* this)
{
	return power_fail_flag;
}

unsigned char check_no_blinking_flag(fsm_t* this)
{
	return (!power_fail_flag);
}

unsigned char check_fast_skip_timer(fsm_t* this)
{
	return is_fast_skip_timer_over();
}


/*FSM Output functions*/

void showtime(fsm_t* this)
{	
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	str_buffer[4]=bcd2char((current_time.year/1000));
	str_buffer[5]=bcd2char(((current_time.year%1000)/100));
	str_buffer[6]=bcd2char(((current_time.year%100)/10));
	str_buffer[7]=bcd2char(current_time.year%10);
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
	
	clear_buffer(str_buffer, 8);
	str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	updateDisplay2(str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);

	updateDisplay3((unsigned char*) getDayOfWeekSpanishNameUppercase8Char(current_time),  0, 0);

	//updateDisplay4(readThermometer(0), 0, updateCogwheel());
	updateDisplay4(printThermistorTemperature(), 0, updateCogwheel());

}

void showtime_and_stop_timer(fsm_t* this)
{
	stop_fast_skip_timer();
	showtime(this);
}	

void updatetime(fsm_t* this)
{
	current_time=getDayOfWeek(current_time); //Updates dayW
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showtime(this);
}

void blinktime(fsm_t* this)
{
	if (1 & current_time.sec)
	{
		showtime(this);
	}
	else 
	{
		clear_buffer(str_buffer, 8);
		updateDisplay1(str_buffer, 0, 0);
		updateDisplay2(str_buffer, 0, 0);
		updateDisplay3(str_buffer, 0, 0);
		updateDisplay4(str_buffer, 0, 0);
	}
}

void clear_blinktime_and_showtime( fsm_t* this )
{
	turn_on_and_blink_display(HT16K33_1_WRITE_ADDRESS, HT16K33_DISPLAY_BLINKING_OFF);
	turn_on_and_blink_display(HT16K33_5_WRITE_ADDRESS, HT16K33_DISPLAY_BLINKING_OFF);
	showtime(this);

}

void clear_oscillator_fault_and_showtime(fsm_t* this)
{
	clear_oscillator_fault(DS3231_WRITE_ADDRESS);
	turn_on_and_blink_display(HT16K33_1_WRITE_ADDRESS, HT16K33_DISPLAY_BLINKING_OFF);
	turn_on_and_blink_display(HT16K33_5_WRITE_ADDRESS, HT16K33_DISPLAY_BLINKING_OFF);
	showtime(this);
}

void blinkhour(fsm_t* this)
{	
	stop_fast_skip_timer();
	if (1 & current_time.sec)
	{
		str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
		str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	}
	else
	{
		str_buffer[0]=' ';
		str_buffer[1]=' ';
	}
	str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	updateDisplay2(str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	str_buffer[4]=bcd2char((current_time.year/1000));
	str_buffer[5]=bcd2char(((current_time.year%1000)/100));
	str_buffer[6]=bcd2char(((current_time.year%100)/10));
	str_buffer[7]=bcd2char(current_time.year%10);
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
}


void modify_hour_once(fsm_t* this)
{
	start_fast_skip_timer(FAST_SKIP_TIMER_DELAY_CYCLES);
	modify_hour(this);
}

void modify_hour(fsm_t* this)
{
	if ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))
	{
		showtime(this);
		return;
	}
	
	if (button_flags & UP_BUTTON_MASK)
	{
		++current_time.hour;
		if (current_time.hour>=HOURS_OF_DAY)
		{
			current_time.hour=0;
		}
		current_time.sec=0;	 
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;		
	}
	if (button_flags & DOWN_BUTTON_MASK)
	{
		if (current_time.hour==0)
		{
			current_time.hour=(HOURS_OF_DAY-1);
		}
		else
		{
			--current_time.hour;
		}
		current_time.sec=0;			 
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showtime(this);	
}


void blinkminute(fsm_t* this)
{
	stop_fast_skip_timer();
	if (FIRST_COLON_MASK & current_time.sec)
	{
		str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
		str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	}
	else
	{
		str_buffer[2]=' ';
		str_buffer[3]=' ';
	}
	str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	updateDisplay2(str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	str_buffer[4]=bcd2char((current_time.year/1000));
	str_buffer[5]=bcd2char(((current_time.year%1000)/100));
	str_buffer[6]=bcd2char(((current_time.year%100)/10));
	str_buffer[7]=bcd2char(current_time.year%10);
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
}

void modify_minute_once(fsm_t* this)
{
	start_fast_skip_timer(FAST_SKIP_TIMER_DELAY_CYCLES);
	modify_minute(this);
}

void modify_minute(fsm_t* this)
{
	if ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))
	{
		showtime(this);
		return;
	}
	
	if (button_flags & UP_BUTTON_MASK)
	{
		++current_time.min;
		if (current_time.min>=MINUTES_OF_HOUR)
		{
			current_time.min=0;
		}
		current_time.sec=0;					 
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	if (button_flags & DOWN_BUTTON_MASK)
	{
		if (current_time.min==0)
		{
			current_time.min=MINUTES_OF_HOUR-1;
		}
		else
		{
			--current_time.min;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showtime(this);
}

void blinkdayM (fsm_t* this)
{
	stop_fast_skip_timer();
	if (FIRST_COLON_MASK & current_time.sec)
	{
		str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
		str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	}
	else
	{
		str_buffer[0]=' ';
		str_buffer[1]=' ';
	}
	str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	str_buffer[4]=bcd2char((current_time.year/1000));
	str_buffer[5]=bcd2char(((current_time.year%1000)/100));
	str_buffer[6]=bcd2char(((current_time.year%100)/10));
	str_buffer[7]=bcd2char(current_time.year%10);
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	updateDisplay2( str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);
}

void modify_dayM_once(fsm_t* this)
{
	start_fast_skip_timer(FAST_SKIP_TIMER_DELAY_CYCLES);
	modify_dayM(this);
}

void modify_dayM(fsm_t* this)
{
	unsigned char daysOfCurrentMonth;
	if ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))
	{
		showtime(this);
		return;
	}
	
	if (is_leap_year(current_time))
	{
		daysOfCurrentMonth=daysOfMonths_leapYear[current_time.month-1];
	}
	else
	{
		daysOfCurrentMonth=daysOfMonths[current_time.month-1];
	}
	
	if (button_flags & UP_BUTTON_MASK)
	{
		++current_time.dayM;
		if (current_time.dayM>daysOfCurrentMonth)
		{
			current_time.dayM=1;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	if (button_flags & DOWN_BUTTON_MASK)
	{
		if (current_time.dayM<=1)
		{
			current_time.dayM=daysOfCurrentMonth;
		}
		else
		{
			--current_time.dayM;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showtime(this);
}

void blinkMonth (fsm_t* this)
{
	stop_fast_skip_timer();
	if (FIRST_COLON_MASK & current_time.sec)
	{
		str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
		str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	}
	else
	{
		str_buffer[2]=' ';
		str_buffer[3]=' ';
	}
	str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	str_buffer[4]=bcd2char((current_time.year/1000));
	str_buffer[5]=bcd2char(((current_time.year%1000)/100));
	str_buffer[6]=bcd2char(((current_time.year%100)/10));
	str_buffer[7]=bcd2char(current_time.year%10);
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	updateDisplay2(str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);

}

void modify_month_once(fsm_t* this)
{
	start_fast_skip_timer(FAST_SKIP_TIMER_DELAY_CYCLES);
	modify_month(this);
}

void modify_month(fsm_t* this)
{
	if ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))
	{
		showtime(this);
		return;
	}
	
	if (button_flags & UP_BUTTON_MASK)
	{
		++current_time.month;
		if (current_time.month>MONTHS_OF_YEAR)
		{
			current_time.month=1;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	if (button_flags & DOWN_BUTTON_MASK)
	{
		if (current_time.month<=1)
		{
			current_time.month=MONTHS_OF_YEAR;
		}
		else
		{
			--current_time.month;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showtime(this);
		return;
	}
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showtime(this);
}


void blinkYear (fsm_t* this)
{
	stop_fast_skip_timer();
	if (FIRST_COLON_MASK & current_time.sec)
	{
		str_buffer[4]=bcd2char((current_time.year/1000));
		str_buffer[5]=bcd2char(((current_time.year%1000)/100));
		str_buffer[6]=bcd2char(((current_time.year%100)/10));
		str_buffer[7]=bcd2char(current_time.year%10);
	}
	else
	{
		str_buffer[4]=' ';
		str_buffer[5]=' ';
		str_buffer[6]=' ';
		str_buffer[7]=' ';
	}
	str_buffer[0]=bcd2char(dec2bcd(current_time.dayM)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.dayM));
	str_buffer[2]=bcd2char(dec2bcd(current_time.month)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.month));
	updateDisplay1(str_buffer, (1<<1) | (1<<3), 0);
	
	str_buffer[0]=bcd2char(dec2bcd(current_time.hour)>>4);
	str_buffer[1]=bcd2char(dec2bcd(current_time.hour));
	str_buffer[2]=bcd2char(dec2bcd(current_time.min)>>4);
	str_buffer[3]=bcd2char(dec2bcd(current_time.min));
	updateDisplay2(str_buffer, 0, (1 & current_time.sec)<<FIRST_COLON_56INCH_SHIFTS);
}

void showYear_and_stop_timer(fsm_t* this)
{
	stop_fast_skip_timer();
	showYear(this);
}

void showYear (fsm_t* this)
{
	showtime(this);
}

void modify_year_once(fsm_t* this)
{
	start_fast_skip_timer(FAST_SKIP_TIMER_DELAY_CYCLES);
	modify_year(this);
}

void modify_year(fsm_t* this)
{
	if ((button_flags & UP_BUTTON_MASK) && (button_flags & DOWN_BUTTON_MASK))
	{
		showYear(this);
		return;
	}
	
	if (button_flags & UP_BUTTON_MASK)
	{
		++current_time.year;
		if (current_time.year>MAX_YEAR)
		{
			current_time.year=MIN_YEAR;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showYear(this);
		return;
	}
	if (button_flags & DOWN_BUTTON_MASK)
	{
		if (current_time.year<=MIN_YEAR)
		{
			current_time.year=MAX_YEAR;
		}
		else
		{
			--current_time.year;
		}
		update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
		showYear(this);
		return;
	}
	update_timestamp_to_RTC(DS3231_WRITE_ADDRESS, current_time, DATA_DECIMAL);
	showYear(this);
}

void startCogWheel()
{
	lastSecondAtCogwheelUpdate = current_time.sec;
	cogwheelLastMask = 1;
}

unsigned char updateCogwheel()
{
	unsigned char currentSec = current_time.sec;
	if (lastSecondAtCogwheelUpdate != currentSec)
	{
		lastSecondAtCogwheelUpdate = currentSec;
		if (cogwheelLastMask >= 128)
		{
			cogwheelLastMask = 1;
		}
		else
		{
			cogwheelLastMask = cogwheelLastMask << 1;
		}

	}
	return cogwheelLastMask;
}

void startThermometer()
{
	(void) readLM75BTemperature_1deg(LM75B_WRITE_ADDRESS);
}

char* readThermometer()
{
	ambientTemperature = readLM75BTemperature_1deg(LM75B_WRITE_ADDRESS);
	signed int temperatureInteger = 0;
	if (ambientTemperature >= -28) // > -0.875
	{
		temperatureInteger = (ambientTemperature >> 8) & 0x00FF;
		if (temperatureInteger / 100 > 0)
		{
			str_buffer[0] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 100));
		}
		else
		{
			str_buffer[0]=' ';
		}
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[1] = bcd2char((dec2bcd((unsigned char) temperatureInteger)) >> 4);
		}
		else
		{
			str_buffer[1] = ' ';
		}

	}
	else
	{
		temperatureInteger = ((ambientTemperature * -1) >> 8) & 0x00FF;
		// Stick - to the first digit
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[0] = '-';
			str_buffer[1] = bcd2char((dec2bcd((unsigned char) temperatureInteger)) >> 4);
		}
		else
		{
			str_buffer[0] = ' ';
			str_buffer[1] = '-';
		}
	}
	str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger));

	str_buffer[3]='º';
	str_buffer[4]='C';
	str_buffer[5]=' ';
	str_buffer[6]=' ';
	str_buffer[7]=' ';
	return str_buffer;
}

char* readThermometerDecimal()
{
	ambientTemperature = readLM75BTemperature_1deg(LM75B_WRITE_ADDRESS);
	signed int temperatureInteger = 0;
	signed int temperatureDecimal = 0;
	if (ambientTemperature < (-0.125 * 32))
	{
		temperatureInteger = ((ambientTemperature * -1) >> 8) & 0x00FF;
		temperatureDecimal = (((ambientTemperature * -1) >> 6) & 0x03) * 25;
		// Stick - to the first digit
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[0] = '-';
			str_buffer[1] = bcd2char((dec2bcd((unsigned char) temperatureInteger)) >> 4);
		}
		else
		{
			str_buffer[0] = ' ';
			str_buffer[1] = '-';
		}
	}
	else
	{
		temperatureInteger = (ambientTemperature >> 8) & 0x00FF;
		temperatureDecimal = ((ambientTemperature >> 6) & 0x03) * 25;
		if (temperatureInteger / 100 > 0)
		{
			str_buffer[0] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 100));
		}
		else
		{
			str_buffer[0]=' ';
		}
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[1] = bcd2char((dec2bcd((unsigned char) temperatureInteger)) >> 4);
		}
		else
		{
			str_buffer[1] = ' ';
		}
	}
	str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger));

	str_buffer[3] = bcd2char((dec2bcd((unsigned char) temperatureDecimal)) >> 4);
	str_buffer[4] = bcd2char(dec2bcd((unsigned char) temperatureDecimal));

	str_buffer[5]='º';
	str_buffer[6]='C';
	str_buffer[7]=' ';
	return str_buffer;
}

char* printThermistorTemperature()
{
	signed int temperatureInteger = (signed int) thermistorTemperature;
	unsigned char temperatureFirstDecimal = ((unsigned char) (thermistorTemperature * 10.0)) % 10;
	if (thermistorTemperature >= 0)
	{
		if (temperatureInteger / 100 > 0)
		{
			str_buffer[0] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 100));
		}
		else
		{
			str_buffer[0]=' ';
		}
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[1] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 10));
		}
		else
		{
			str_buffer[1] = ' ';
		}
		str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger % 10));
	}
	else
	{
		temperatureInteger *= -1;
		// Stick - to the first digit
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[0] = '-';
			str_buffer[1] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 10));
		}
		else
		{
			str_buffer[0] = ' ';
			str_buffer[1] = '-';
		}
		str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger % 10));
	}
	str_buffer[3]='º';
	str_buffer[4]='C';
	str_buffer[5]=' ';
	str_buffer[6]=' ';
	str_buffer[7]=' ';
	return str_buffer;
}

char* printThermistorTemperatureDecimal()
{
	signed int temperatureInteger = (signed int) thermistorTemperature;
	unsigned char temperatureFirstDecimal = ((unsigned char) (thermistorTemperature * 10.0)) % 10;
	// Same as integer by now
	if (thermistorTemperature >= 0)
	{
		if (temperatureInteger / 100 > 0)
		{
			str_buffer[0] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 100));
		}
		else
		{
			str_buffer[0]=' ';
		}
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[1] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 10));
		}
		else
		{
			str_buffer[1] = ' ';
		}
		str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger % 10));
	}
	else
	{
		temperatureInteger *= -1;
		// Stick - to the first digit
		if (temperatureInteger / 10 > 0)
		{
			str_buffer[0] = '-';
			str_buffer[1] = bcd2char(dec2bcd((unsigned char) temperatureInteger / 10));
		}
		else
		{
			str_buffer[0] = ' ';
			str_buffer[1] = '-';
		}
		str_buffer[2] = bcd2char(dec2bcd((unsigned char) temperatureInteger % 10));
	}
	str_buffer[3]='º';
	str_buffer[4]='C';
	str_buffer[5]=' ';
	str_buffer[6]=' ';
	str_buffer[7]=' ';
	return str_buffer;
}

double readThermistor (unsigned char rawReading)
{
	double resistance = THERMISTOR_SERIES_RESISTOR_OHMS * ((double) rawReading / (255.0 - (double) rawReading));
	double steinhart = resistance / THERMISTOR_NOMINAL_OHMS;     // (R/Ro)
	steinhart = log(steinhart);                  // ln(R/Ro)
	steinhart /= THERMISTOR_BETA;                   // 1/B * ln(R/Ro)
	steinhart += (1.0 / (THERMISTOR_TEMPERATURE_NOMINAL)); // + (1/To)
	steinhart = 1.0 / steinhart;                 // Invert
	return steinhart;
}

unsigned char updateDisplay1(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	unsigned char updateFlag = 0;
	for (unsigned char i = 0; i < 8; ++i)
	{
		if (str[i] != display1_buffer[i])
		{
			updateFlag = 0xFF;
			break; // To avoid testing the whole buffer when a mismatch occurs
		}
	}
	if (decimal_dots_mask != display1_decimal_dots_buffer ||  special_dots_mask != display1_special_dots_buffer)
	{
		updateFlag = 0xFF;
	}
	if (updateFlag)
	{
		for (unsigned char i = 0; i < 8; ++i)
		{
			display1_buffer[i] = str[i];
		}
		display1_decimal_dots_buffer = decimal_dots_mask;
		display1_special_dots_buffer = special_dots_mask;
		display_update(HT16K33_1_WRITE_ADDRESS, DISPLAY_3942BG, display1_buffer, display1_decimal_dots_buffer, display1_special_dots_buffer);
	}
	return updateFlag;
}

unsigned char updateDisplay2(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	unsigned char updateFlag = 0;
	for (unsigned char i = 0; i < 8; ++i)
	{
		if (str[i] != display2_buffer[i])
		{
			updateFlag = 0xFF;
			break; // To avoid testing the whole buffer when a mismatch occurs
		}
	}
	if (decimal_dots_mask != display2_decimal_dots_buffer ||  special_dots_mask != display2_special_dots_buffer)
	{
		updateFlag = 0xFF;
	}
	if (updateFlag)
	{
		for (unsigned char i = 0; i < 8; ++i)
		{
			display2_buffer[i] = str[i];
		}
		display2_decimal_dots_buffer = decimal_dots_mask;
		display2_special_dots_buffer = special_dots_mask;
		display_update(HT16K33_5_WRITE_ADDRESS, DISPLAY_SC15, display2_buffer, display2_decimal_dots_buffer, display2_special_dots_buffer);
	}
	return updateFlag;
}

unsigned char updateDisplay3(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	unsigned char updateFlag = 0;
	for (unsigned char i = 0; i < 8; ++i)
	{
		if (str[i] != display3_buffer[i])
		{
			updateFlag = 0xFF;
			break; // To avoid testing the whole buffer when a mismatch occurs
		}
	}
	if (decimal_dots_mask != display3_decimal_dots_buffer ||  special_dots_mask != display3_special_dots_buffer)
	{
		updateFlag = 0xFF;
	}
	if (updateFlag)
	{
		for (unsigned char i = 0; i < 8; ++i)
		{
			display3_buffer[i] = str[i];
		}
		display3_decimal_dots_buffer = decimal_dots_mask;
		display3_special_dots_buffer = special_dots_mask;
		display_update(HT16K33_3_WRITE_ADDRESS, DISPLAY_PDA54_14SEGMENTS, display3_buffer, display3_decimal_dots_buffer, display3_special_dots_buffer);
	}
	return updateFlag;
}

unsigned char updateDisplay4(unsigned char* str, unsigned char decimal_dots_mask, unsigned char special_dots_mask)
{
	unsigned char updateFlag = 0;
	for (unsigned char i = 0; i < 8; ++i)
	{
		if (str[i] != display4_buffer[i])
		{
			updateFlag = 0xFF;
			break; // To avoid testing the whole buffer when a mismatch occurs
		}
	}
	if (decimal_dots_mask != display4_decimal_dots_buffer ||  special_dots_mask != display4_special_dots_buffer)
	{
		updateFlag = 0xFF;
	}
	if (updateFlag)
	{
		for (unsigned char i = 0; i < 8; ++i)
		{
			display4_buffer[i] = str[i];
		}
		display4_decimal_dots_buffer = decimal_dots_mask;
		display4_special_dots_buffer = special_dots_mask;
		display_update(HT16K33_4_WRITE_ADDRESS, DISPLAY_DVD, display4_buffer, display4_decimal_dots_buffer, display4_special_dots_buffer);
	}
	return updateFlag;
}



void InitADC(void)
{
	// Select Vref=AVcc & Left justified
	ADMUX |= (1<<REFS0) | (1<<ADLAR);
	//set prescaler to 128 and enable ADC
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

unsigned char ReadADC(unsigned char ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADCH;
}



	
