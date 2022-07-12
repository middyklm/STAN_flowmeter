/*****************************************************
 * main.c * for BUOYANCY 
 * *
 * PURPOSE *
 * *
 *****************************************************/


/*

FUSES:
Extended	0xFF
High		0x91
Low			0xE2


Libraries need to be added:
libm
libm.a
libprintf_flt.a

NOTE: if the LCD is enabled the board will not work if there is no LCD connected


TODO:
need to take motor driver code out of main to make it accessible to buoyancy code
Need to add ifndef to all header files to prevent them calling infinitely


make the manual mode commands more informative, show the t format in the prompt
write pause code - maybe a while loop in each part of the sampling code which removes the run bit in the modeflags and loops until the run butting is repressed?
make code to adapt code for max pressre (sterivex or 45mm) in thwe command line


Style guide as per NASA C style guide 1994 
Functions in PascalCamelCase
variables_lower_case_with_underscores
CONSTANTSINCAPS
1TBS bracketing style, nested }ELSE{ statements



Pin Number	Pin Name

98	AREF    NC              
100	AVCC    tied to +3V3
11	GND
32	GND
62	GND
81	GND
99	GND
78	PA0 ( AD0 )         nc
77	PA1 ( AD1 )         nc
76	PA2 ( AD2 )         nc
75	PA3 ( AD3 )         nc
74	PA4 ( AD4 )         nc
73	PA5 ( AD5 )        output - pres1_ss output to select pressure sensor 1 (SPI)
72	PA6 ( AD6 )        output - ESP_CH_PD  output ESP8266 power-down mode (CH-PD low)
71	PA7 ( AD7 )        output - ESP reset
19	PB0 ( SS/PCINT0 )    nc
20	PB1 ( SCK/PCINT1 )   output
21	PB2 ( MOSI/PCINT2 )  output
22	PB3 ( MISO/PCINT3 )  input - spi bus
23	PB4 ( OC2A/PCINT4 )  nc
24	PB5 ( OC1A/PCINT5 )   nc
25	PB6 ( OC1B/PCINT6 )   nc
26	PB7 ( OC0A/OC1C/PCINT7 )   nc
53	PC0 ( A8 )   
54	PC1 ( A9 )
55	PC2 ( A10 )
56	PC3 ( A11 )
57	PC4 ( A12 )
58	PC5 ( A13 )
59	PC6 ( A14 )      ESP_GPIO2
60	PC7 ( A15 )      ESP_GPIO0
43	PD0 ( SCL/INT0 )  SCL
44	PD1 ( SDA/INT1 )     SDA
45	PD2 ( RXDI/INT2 )    
46	PD3 ( TXD1/INT3 )
47	PD4 ( ICP1 )
48	PD5 ( XCK1 )
49	PD6 ( T1 )
50	PD7 ( T0 )
2	PE0 ( RXD0/PCINT8 )    RX_USART0
3	PE1 ( TXD0 )           TX_USART0
4	PE2 ( XCK0/AIN0 )
5	PE3 ( OC3A/AIN1 )
6	PE4 ( OC3B/INT4 )
7	PE5 ( OC3C/INT5 )
8	PE6 ( T3/INT6 )
9	PE7 ( CLKO/ICP3/INT7 )
97	PF0 ( ADC0 )       output -  motor_driver2_in1
96	PF1 ( ADC1 )       output -  motor_driver2_in2
95	PF2 ( ADC2 )       output -  motor_driver2_pwm
94	PF3 ( ADC3 )       output -  motor_driver2_stby
93	PF4 ( ADC4/TMK )   output -  motor_driver1_in1
92	PF5 ( ADC5/TMS )   output -  motor_driver1_in2
91	PF6 ( ADC6 )       output -  motor_driver1_pwm
90	PF7 ( ADC7 )       output -  motor_driver1_stby
51	PG0 ( WR )
52	PG1 ( RD )
70	PG2 ( ALE )       output - LED5
28	PG3 ( TOSC2 )     output - FET_G3
29	PG4 ( TOSC1 )     output - FET_G4
1	PG5 ( OC0B )
12	PH0 ( RXD2 )
13	PH1 ( TXD2 )
14	PH2 ( XCK2 )      output - PRES_sensor_supply
15	PH3 ( OC4A )
16	PH4 ( OC4B )
17	PH5 ( OC4C )
18	PH6 ( OC2B )
27	PH7 ( T4 )
63	PJ0 ( RXD3/PCINT9 )      output - ESP_TX
64	PJ1 ( TXD3/PCINT10 )     input - ESP_RX
65	PJ2 ( XCK3/PCINT11 )     output - LED_MANUAL
66	PJ3 ( PCINT12 )          output - LED1
67	PJ4 ( PCINT13 )          output - LED2
68	PJ5 ( PCINT14 )          output - LED3
69	PJ6 ( PCINT 15 )         output - LED4
79	PJ7
89	PK0 ( ADC8/PCINT16 )
88	PK1 ( ADC9/PCINT17 )
87	PK2 ( ADC10/PCINT18 )       input - sw2_pin 
86	PK3 ( ADC11/PCINT19 )       input - sw1_pin
85	PK4 ( ADC12/PCINT20 )       input - sw0_pin
84	PK5 ( ADC13/PCINT21 )
83	PK6 ( ADC14/PCINT22 )
82	PK7 ( ADC15/PCINT23 )
35	PL0 ( ICP4 )
36	PL1 ( ICP5 )
37	PL2 ( T5 )
38	PL3 ( OC5A )
39	PL4 ( OC5B )              output - FET_G2
40	PL5 ( OC5C )              output - FET_G1
41	PL6
42	PL7
30	RESET
10	VCC
31	VCC
61	VCC
80	VCC
34	XTAL1          nc
33	XTAL2          nc



*/



// ********************************************************************************
// Includes
// ********************************************************************************
#include "config.h"
#ifndef F_CPU
#define F_CPU 8000000UL // this is placed above delay/h so that f_CPU is defines before delay.h loads
#endif
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <string.h>				// memcpy, memset
#include <math.h>
#include <stdlib.h>  // exit
#include <avr/pgmspace.h>  // Used by lcd.h and to store manual comands
#include "uart.h"
#include "i2c.h"
#include "DS3231.h"
#include "lcd.h"
#include "logging.h"
#include "fram.h"
#include "wakeupINT2.h"
#include "fs2012.h"
#include "motorDriver.h"
#include "buoyancy.h"
#include "spi.h"
#include "pressureSensor.h"
#include "r9valve.h"
#include "menu_text.h"
#include "settings.h"
#include "volume.h"



// ********************************************************************************
// Macros and Defines
// ********************************************************************************


#define firmware_version "0.0.1"
#define TRUE		1
#define FALSE		0
#define ON			1
#define OFF			0
#define CHAR_NEWLINE '\n'
#define CHAR_RETURN '\r'
#define RETURN_NEWLINE "\r\n"
#define SUPPLYVOLTAGE 3.3
#define NUM_OF_COMMANDS_IN_ARRAY 50 // this must be exact, if the program searches for a non existent command it will crash

// SAMPLING PROTOCOL DEFINES
#define PRESERVATIVE_PUMP_TIMEOUT_SECONDS 60
#define INLET_WASH_TIMEOUT_SECONDS 15  // 8 seconds seems to work here
#define SAMPLE_PUMP_TIMEOUT_SECONDS 3000 // pump will stop if pressure is not reached after 50 minutes of pumping
#define SAMPLE_MAX_VOLUME_mL 1000 // pump will stop once 1000 mL has passed through flow sensor
#define NUMBER_OF_MODES 7 // zero indexed so "NUMBER_OF_MODES=7" is actually 8 different modes 
#define BAG_PRESSURE_CUTOFF_PSI 16.00			//float: defines the pressure in PSI at which the peristaltic pump will stop pumping when the bag is deemed to be full based on pressure
/* 80 psi = 5.5 bar and is the max rating for the millipore filter housing, sterivex filters must be to a max 45psi /3.1bar, this needs to be the max filter pressure rating plus 15 PSI 
as the Honeywell sensors give absolute pressure measurements not gauge pressure measurements */
#define MAX_FILTER_PRESSURE_PSI 60.00	
#define SAMPLE_PUMP_STARTING_PWM_SPEED 255 // the 8 bit pwm value that the pump starts at, this will likely be unchanged at full speed(255)
#define MINIMUM_SAMPLE_PUMP_PWM_SPEED 120  // the PWM (8 bit/ 0-255) value below which the sample
#define MIN_SAMPLE_TIME_SECONDS 180		// sample at least for 180 seconds event when pressure is higher then MAX_FILTER_PRESSURE_PSI
#define DATA_RECORD_INTERVAL 5		// the interval between data points during sampling being recorded to FRAM and printf

// WASH PROTOCOL DEFINES
#define NUMBER_OF_WASH_STEPS 4
#define WASH_RUNTHROUGH_TIMEOUT_SECONDS 15

// PUSH BUTTON DEFINES
#define PUSH_BUTTON_PORT	PORTE    // defines the port that the push buttons, via schmitt trigger, are connected to 
#define PUSH_BUTTON_DDR		DDRE	// defines the Data direction register for the push button pins
#define PUSH_BUTTON_0_PIN	PB4		//defines the microcontroller pins that the push buttons are connected to 
#define PUSH_BUTTON_1_PIN	PB5
#define PUSH_BUTTON_2_PIN	PB6




// LED DEFINES
#define LED_MANUAL_MODE	0		// same as LED_0 but alternate name to make code easier to read
#define LED_TEST_MODE	1
#define LED_SAMPLE_MODE	2	
#define LED_CLEANING_MODE	3	
#define LED_PRIMING_MODE	4		
//#define LED_PORT			PORTJ


#define LED_0_PIN			PK7
#define LED_0_PORT			PORTK
#define LED_0_DDR			DDRK

#define LED_1_PIN			PK6
#define LED_1_PORT			PORTK
#define LED_1_DDR			DDRK

#define LED_2_PIN			PK5
#define LED_2_PORT			PORTK
#define LED_2_DDR			DDRK

#define LED_3_PIN			PK4
#define LED_3_PORT			PORTK
#define LED_3_DDR			DDRK

#define LED_4_PIN			PK3
#define LED_4_PORT			PORTK
#define LED_4_DDR			DDRK

#define LED_5_PIN			PF3
#define LED_5_PORT			PORTF
#define LED_5_DDR			DDRF

#define LED_6_PIN			PF1
#define LED_6_PORT			PORTF
#define LED_6_DDR			DDRF

#define LED_7_PIN			PG5
#define LED_7_PORT			PORTG
#define LED_7_DDR			DDRG

// thes alternate definitions need to be corrected for stan5
#define MANUAL_MODE_LED_PIN		LED_0_PIN
#define MANUAL_MODE_LED_PIN		LED_0_PIN
#define MANUAL_MODE_LED_PIN		LED_0_PIN

#define TEST_MODE_LED_PIN		LED_1_PIN
#define TEST_MODE_LED_PIN		LED_1_PIN
#define TEST_MODE_LED_PIN		LED_1_PIN

#define SAMPLE_MODE_LED_PIN		LED_2_PIN
#define SAMPLE_MODE_LED_PIN		LED_2_PIN
#define SAMPLE_MODE_LED_PIN		LED_2_PIN

#define CLEANING_MODE_LED_PIN	LED_3_PIN
#define CLEANING_MODE_LED_PIN	LED_3_PIN
#define CLEANING_MODE_LED_PIN	LED_3_PIN

#define PRIMING_MODE_LED_PIN	LED_4_PIN
#define PRIMING_MODE_LED_PIN	LED_4_PIN
#define PRIMING_MODE_LED_PIN	LED_4_PIN

// INTERRUPT DEFINES
#define RUN_BUTTON_INT		INT5_vect		/**/
#define MODE_BUTTON_INT		INT4_vect		/**/
#define RTC_INT				INT7_vect		/**/



// ********************************************************************************
// Function Prototypes
// ********************************************************************************

// pwm function prototypes
//void pwm_init(void);
//void PH6_pwm_set(uint8_t pwm_value);
//void PB7_pwm_set(uint8_t pwm_value);
void pwm_init_PL5();	//pwm for valve 1
void pwm_init_PL3();	//pwm for motor_driver_1_pwm


void copy_command (void);
//unsigned long parse_assignment (); //this was removed as the code seems to work without it, double check this************************
void processCommand(char command_string[]);
int collectSample(void);
int collectSampleAdaptivePressure(void);
int sampleTest(void);
void commandTest(char command_str[]);
void printFirmwareVersion(char command_str[]);
int usart_putchar_printf(char var, FILE *stream);  // printf function

void test(char command_value_string_passed[]);


void init_sleep_mode(void);
void sleep_mode_on(void);

// push buttons function protoypes
void init_push_buttons(void);

// LED controls
void Ledinit(void);
void LedBlink(uint8_t blink_duration_in_seconds);
void LedControl (uint8_t led_number, uint8_t on_off);
void LedAllOFF(void);

//lcd functions
void lcdStatusLineUpdate(const char statusText[]);
void lcdPrintAllLines(const char line0[],const char line1[],const char line2[],const char line3[]);
void lcdGetModeAndUpdate();  // updates the top lcd line with the current mode
void lcdLine2Update(const char infoText[]);
void lcdClearLine(uint8_t line);
void lcdLine0Update(const char line0text[20]);
void lcdLine1Update(const char line1Text[20]);  // clears and adds text to lcd line 1 (second line down)
void lcdLine2Update(const char line2text[20]);
void lcdLine3Update(const char line3text[20]);
void lcdUpdateSamplingTime();
void lcdUpdatePWMValue(uint8_t pwm_value);
void lcdUpdatePressureInfo(double filter_pressure,double bag_pressure);
void lcdShowIfRunCompleted();

void LogModeFlags();
void printModeflags();



// ********************************************************************************
// Global Variables
// ********************************************************************************
/*
//modeFlags variable holds the state of the device. first 3 LSBs (bit 0...2)are the current operating mode. bit 3 (run bit ) is if the mode is active, which is when the start button is pushed
modes. The MSB (bit 7) is set when the device is ready to collect a sample (i.e the sample bag is empty) once a sample has been taken the bit is cleared and further sampling cannot be triggered 
000 sleep - awaits external trigger for sampling
001 manual - allows uart serial commands to be inputted
010 test - test main components and send results via serial
011 sample - performs the sampling operation	
100 clean - performs decontamination
101 priming mode - 
110 UART input mode - sets the modeflags to listen to uart, this was written to allow sountrap input to wake the stan pcb
111 - fram mode

*/
volatile unsigned char modeFlags = 0b10000000; // when initialised the MSB (ready to sample) bit is set
// The inputted commands are never going to be
// more than 8 chars long.
//volatile so the ISR can alter them
volatile unsigned char data_in[9];
volatile unsigned char command_in[9];
volatile unsigned char data_count;
volatile unsigned char command_ready;
volatile unsigned char INT2flag = 0, usart1Stop = 0;
unsigned char command_valid;
//unsigned int sensitivity;
char command_name[4];  // used by the processCommand function, made global as it was not working in local, i don't know why!


/*** constant definitions for LCD****/
static const PROGMEM unsigned char copyRightChar[] =
{
	0x07, 0x08, 0x13, 0x14, 0x14, 0x13, 0x08, 0x07,
	0x00, 0x10, 0x08, 0x08, 0x08, 0x08, 0x10, 0x00
};


// ********************************************************************************
// type defs & Structures
// ********************************************************************************


typedef void (*functionPointerType)(void); // declares functionPointerType as a type that can be used in the struct "command_structure
struct command_structure{   // declares a structure definition which can hold a command string, its function pointer and a help string;
	char const *name;
	functionPointerType execute;
	char const *help;
};


// ********************************************************************************
// Structures definitions
// ********************************************************************************
// commands 0-9
const char helptext0[] PROGMEM ="helptext 0 is here for testing";
const char helptext1[] PROGMEM ="b";
const char helptext2[] PROGMEM ="c";
const char helptext3[] PROGMEM ="a";
const char helptext4[] PROGMEM ="b";
const char helptext5[] PROGMEM ="c";
const char helptext6[] PROGMEM ="a";
const char helptext7[] PROGMEM ="b";
const char helptext8[] PROGMEM ="c";
const char helptext9[] PROGMEM ="a";
// commands 10-19
const char helptext10[] PROGMEM ="a";
const char helptext11[] PROGMEM ="b";
const char helptext12[] PROGMEM ="c";
const char helptext13[] PROGMEM ="clock time set - sets the RTC from a string \"hhmmss\"";
const char helptext14[] PROGMEM ="clock date set - sets the clock date from string \"ddmmyy\"";
const char helptext15[] PROGMEM ="clock alarm1 print";
const char helptext16[] PROGMEM ="clock alarm1 set as <hhmmssDD>";  

const char helptext17[] PROGMEM ="clock alarm on";
const char helptext18[] PROGMEM ="clock alarm off";
const char helptext19[] PROGMEM ="a";
// commands 20-29
const char helptext20[] PROGMEM ="a";
const char helptext21[] PROGMEM ="b";
const char helptext22[] PROGMEM ="c";
const char helptext23[] PROGMEM ="a";
const char helptext24[] PROGMEM ="b";
const char helptext25[] PROGMEM ="c";
const char helptext26[] PROGMEM ="a";
const char helptext27[] PROGMEM ="b";
const char helptext28[] PROGMEM ="c";
const char helptext29[] PROGMEM ="a";
// commands 30-39
const char helptext30[] PROGMEM ="a";
const char helptext31[] PROGMEM ="b";
const char helptext32[] PROGMEM ="c";
const char helptext33[] PROGMEM ="a";
const char helptext34[] PROGMEM ="b";
const char helptext35[] PROGMEM ="c";
const char helptext36[] PROGMEM ="a";
const char helptext37[] PROGMEM ="b";
const char helptext38[] PROGMEM ="c";
const char helptext39[] PROGMEM ="a";
// commands 40-49
const char helptext40[] PROGMEM ="a";
const char helptext41[] PROGMEM ="b";
const char helptext42[] PROGMEM ="c";
const char helptext43[] PROGMEM ="a";
const char helptext44[] PROGMEM ="b";
const char helptext45[] PROGMEM ="c";
const char helptext46[] PROGMEM ="a";
const char helptext47[] PROGMEM ="b";
const char helptext48[] PROGMEM ="c";
const char helptext49[] PROGMEM ="end";


const struct command_structure command_structure_array[NUM_OF_COMMANDS_IN_ARRAY] PROGMEM = {        // all commands are stored here, if list length is changes must be changed in #define NUM_OF_COMMANDS_IN_ARRAY 50
	// commands 0-9
	{"tst", (void(*)(void))&sampleTest, helptext0},
	{"run", (void(*)(void))&collectSample, helptext1},
	{"tmp", (void(*)(void))&printRTCtemp, helptext2},
	{"cdp", (void(*)(void))&printDatestring, helptext3},
	{"ctp", (void(*)(void))&printTimestring, helptext4},
	{"blank", 0, helptext5},
	{"blank", 0, helptext6},
	{"blank", 0, helptext7},
	{"blank", 0, helptext8},
	{"blank", 0, helptext9},
	//commands 10-19  clock commands
	{"blank", 0, helptext10},
	{"blank", 0, helptext11},
	{"blank", 0, helptext12},
	{"cts", (void(*)(void))&setClockfromString, helptext13 },
	{"cds", (void(*)(void))&setDatefromString, helptext14 },
	{"cap", (void(*)(void))&printAlarm1string, helptext15},
	{"cas", (void(*)(void))&setAlarm1string, helptext16},
	{"cao", (void(*)(void))&ds3231Alarm1on, helptext17},
	{"caf", (void(*)(void))&ds3231Alarm1off, helptext18},
	{"blank", 0, helptext19},
	//commands 20-29
	{"blank", 0, helptext20},
	{"blank", 0, helptext21},
	{"blank", 0, helptext22},
	{"blank", 0, helptext23},
	{"blank", 0, helptext24},
	{"blank", 0, helptext25},
	{"blank", 0, helptext26},
	{"blank", 0, helptext27},
	{"blank", 0, helptext28},
	{"blank", 0, helptext29},
	//commands 30-39
	{"blank", 0, helptext30},
	{"blank", 0, helptext31},
	{"blank", 0, helptext32},
	{"blank", 0, helptext33},
	{"blank", 0, helptext34},
	{"blank", 0, helptext35},
	{"blank", 0, helptext36},
	{"blank", 0, helptext37},
	{"blank", 0, helptext38},
	{"blank", 0, helptext39},
	//commands 40-49 (total of 50 commands) Test commands
	{"blank", 0, helptext40},
	{"blank", 0, helptext41},
	{"blank", 0, helptext42},
	{"blank", 0, helptext43},
	{"blank", 0, helptext44},
	{"blank", 0, helptext45},
	{"blank", 0, helptext46},
	{"blank", 0, helptext47},
	{"blank", 0, helptext48},
	{"end",   0, helptext49},
};  // holds the command list


settings_Union settings;
struct vol v;
// ********************************************************************************
// Interrupts
// ********************************************************************************

// place the TIMERx ISR body here
#include "timerx_isr.h"

// INT2 is the RTC 

ISR (INT2_vect){
	EIMSK &= ~(1 << INT2); // External Interrupt Mask Register - Clear INT2 enable bit before to prevent the RTC from continually triggering INT2
	uart_puts(0,"INT2 triggered\r\n");    //  ***TEST CODE comment out on deployment
	modeFlags &= 0b11111000; /*clear the last 3 LSBits */
	modeFlags |= 0b00000110; // select UART mode// AVR enters UART input mode, 
	USART1_re_enable();		// enable UART1 for communication with SoundTrap, at the same time, disable INT2
}



/*	This INT is triggered by the RTC alarm and beings the sampling process	*/
ISR (RTC_INT){
	EIMSK &= ~(1 << INT7);	// External Interrupt Mask Register - Clear INT7 enable bit to prevent constant INteerupt as the RTC stays low level and the sense control interrupts do not work from sleep so for this application the INT& must be turned off to  */
	_delay_ms (500);  // this delay stops the int re triggering
	LogDebug("INT7 RTC_INT triggered\r\n");
	//uart_puts(0, "INT7 RTC_INT triggered\r\n");    /*  ***TEST CODE comment out on deployment */
	modeFlags &= 0b11111000; /*clear the last 3 LSBits */
	modeFlags |= 0b00001011;  /* Places the sampler into sample mode  011 with run bit (bit3) active*/
}



/*	This INT toggles the run/start button flags */
ISR (RUN_BUTTON_INT){  
	_delay_ms (500);  // this delay stops the int re triggering
	LogDebug("INT5 triggered\r\n");
	LogDebug("run button pressed\r\n");
	//uart_puts(0, "INT5 triggered\r\n");    //  ***TEST CODE comment out on deployment
	if ((modeFlags & 0b00001000) == 0b00000000) {// if run/start bit is low, set to high
		modeFlags |= 0b00001000;	// set run/start active
		LogDebug("run/start mode on\r\n");
		//printf("run/start mode on\r\n");    //  ***TEST CODE comment out on deployment
	}
	else {									// if run/start active, set to low
		modeFlags &= ~(0b00001000);	
		LogDebug("run/start mode off\r\n");
		//printf("run/start mode off\r\n");    //  ***TEST CODE comment out on deployment
	}
	printModeflags();
}
	

/*	This INT is triggered by the mode button, cycles between mode 0...NUMBER_OF_MODESINT 	*/
ISR(MODE_BUTTON_INT) {  
	_delay_ms (500); //this delay stops the int re triggering
	//uart_puts(0," MOBI sample mode button pressed \r\n**INT7 ISR Triggered\r\n");    //  ***TEST CODE comment out on deployment
	LogDebug("Mode button pressed\r\n");
	modeFlags &= ~(0b00001000);	//clear the mode active/start bit so it doesnt trigger when going into a new mode
	if ((modeFlags & 0b00000111) < NUMBER_OF_MODES ){  // if the modeflags is less than the max number of modes
		modeFlags += 0b00000001;	//increment to the next mode
	}
	else if ((modeFlags & 0b00000111) == NUMBER_OF_MODES ){		// if the max number of modes has been reached
		modeFlags &= ~(0b00000111);		// return the 3LSB to 0b000
	}
 	printModeflags();
}
 
ISR (USART0_RX_vect){
	//uart_puts(0," USART0_RX_vect triggered\r\n");    //  ***TEST CODE comment out on deployment - DO NOT USE!!! during operation, interferes with data_in string
	// Get data from the USART in register, cycles through each input and puts it into an array
	data_in[data_count] = UDR0;
	// End of line!
	if (data_in[data_count] == '\n') {
		command_ready = TRUE;
		// Reset to 0, ready to go again
		data_count = 0;
		// 		uart_puts(0,"UART_RX ISR found end of a command (\n) command = ");			//  ***TEST CODE comment out on deployment
		// 		uart_puts(0,data_in);
		// 		uart_puts(0,"\r\n");
		//printf("USART INT data_in = %s",data_in);
	}
	else {
		data_count++;
	}
}

// include pulse flow ISR implementation here
#include "pulseflow_isr.h"

// Print settings
void settings_Print(void)
{
	printf("---- Settings ----\r\n");
	printf("Sample volume: %u ml\r\n", settings.volumeSample);
	printf("----\r\n");
}


// ********************************************************************************
// Main
// ********************************************************************************
int main(void){
	
	// -------- Inits --------- //	
    init_UART(0);//USART init, this is used for pc commuication via the puts and printf functions
	uart_puts(0, "\r\nOneShot  Startup Sequence\r\n");//startup text to UART
	uart_puts(0, "Firmware Version  ");
	uart_puts(0, firmware_version);
	uart_puts(0, "\r\n\r\n");
		
	// these have been placed here to see if supplying power to the pressure sensor earlier will fix the problem of it now working, they come from the init spi function
// PRESSURE_SENSOR1_DDR |= (1<<PRESSURE_SENSOR1_PIN); // sets pressure sensor CS/SS (chip select) pins to output (pressuresensor1)
// PRESSURE_SENSOR1_PORT |=  (1<<PRESSURE_SENSOR1_PIN); // sets SPI chip select pins for pressuresensor1 to HIGH to deassert them until requir	
	/* initialize display, cursor off */
	#ifdef LCD_ENABLE
		lcd_init(LCD_DISP_ON);
	#endif
	//init_push_buttons(); // sets pullups on the active low schmitt trigger 
	Ledinit();  //init peripherals etc
	initI2C();
	//InitLogging(); // defaults to VERBOSE logging (in logging.c global variable), this will need to be user modifiable with software and hardware eventually	
	DS3231_init();
	PressureSensorInit();	
	spi_init_master();
	pwm_init_PL5();	// initialise pwm for the valve driver
	pwm_init_PL3();  // initialise pwm for the motor driver 
	init_sleep_mode();
	init_interrupts();	// this has been moved up from immediately before the sleep while loop below,if there are any interrupt issues  this may need to go back down 
	MotordriverInit();	// initialize motor driver pins as outputs
	settings = settings_Load();
	settings_Print();
	v = volume_init(v);
	
	#ifdef LCD_ENABLE
		lcd_init(LCD_DISP_ON); /* initialize LCD display, cursor off */
	#endif
	//INT2_init();		// Initialise the interrupt INT2 code. This is used to send wakeup signals from SoundTrap to AVR via RX1/TNT2
	LedBlink(1); // blinks for x seconds to show activity
	
 	#ifndef LOGGING_ENABLE  // if logging is not enables, print the time date and alarm. if logging is enables then this will be appended to each message
 		printTimestring();
 		printDatestring();
 		printAlarm1string();
	#endif
	
	#ifdef FRAM_ENABLE
		initFRAM();			/* Initialise FRAM. Read last written memory address*/
	#endif
	
	printModeflags();

	//fram_write_sequential_address_to_fram();
	//fram_return_sequential_address();


	//startup text to LCD
	#ifdef LCD_ENABLE
		lcdStatusLineUpdate("Initialising");
	#endif
	
	// turn off  all LEDs
	LedAllOFF();
	
	//////////////////////////////////////////////////////////
	#ifdef DEMO_FLOW_RATE_SENSOR
		#warning "compiling flow demo, not production binary"
		//paddle_sensor_start();
		//timerx_start();
		flow_sensor_start(); 
		sec_subsec_t t;

		// Header H5 pins 2, 4 and 6
		//extern counter_t counter_11;
		//extern counter_t counter_12;
		//extern counter_t counter_13;
		
		//while(1){
			//sec_subsec_t now;
			//timerx_time(&now);
			//if(now.sub !=t.sub){
				//t = now;
				//printf("%3lu.%u\t",t.sec,t.sub);
				//printf ("%3u\t",counter_11.pps_last);
				//printf ("%5lu\t",counter_11.pcint_total);
				//printf ("%3u\t",counter_12.pps_last);
				//printf ("%5lu\t",counter_12.pcint_total);
				//printf ("%3u\t",counter_13.pps_last);
				//printf ("%5lu\t",counter_13.pcint_total);
				//printf("\n");
				//
			//}
		//}
		
		//printf(" time\trate\ttotal\tmL\n");
		//while(1){
			//sec_subsec_t now;
			//timerx_time(&now);
			//if(now.sub !=t.sub){
				//t = now;
				//printf("%3lu.%u\t",t.sec,t.sub);
				//printf ("%3u\t",paddle_rate_now());        // flow rate now
				//printf ("%5lu\t",paddle_net_impulses());
				//printf ("%5lu \n",paddle_net_uL()/1000);
			//
			//}
		//}
		
		printf(" time  rate accum uLtot\n");
		while(1){
			timerx_time(&t); 
			printf("%3lu.%u ",t.sec,t.sub);
		
			//flow_snapshot(); // snapshot volatile variables
			printf ("%5u ",flow_rate_now());        // flow rate now (65535 = invalid)
			printf ("%5lu ",flow_net_unitsubsec()); // accumulated unit x subseconds
			printf ("%5lu\n",flow_net_uL());        // accumulated microlitres
		}
	#endif
	//////////////////////////////////////////////////////////

	pin_change_1_init();	
	char c = 0, oldChar = 0;

	while (1){
		// goes to sleep on startup or when no other mode is selected
		LedAllOFF();
		#ifdef LCD_ENABLE
			lcdPrintAllLines("Sleep Mode:","Press mode key to","wake","");  // print the first two lines of the lcd
		#endif
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		//uart_puts(0,"Sleep mode ON\r\n"); // deprecated code, replaced with logging code
		_delay_ms(30);
		sleep_mode_on();
		//sleep_mode();
		//when woken from sleep: turn off sleep enable and check if sleep was interrupted by manual mode or by auto sampling request
		//fram_memory_report();

		getinput:
		if (PCINT8flag)	{ // AVR wakes up if pin changed in RX0 (uart0)
			PCINT8flag = false;			// Reset interrupt flag			
			puts_P(MAINMENUTEXT); 
			c = USART0_getchar();
			uart_puts(0,"Entered: ");
			USART0_send(c); uart_puts(0,"\r\n");
		}
	
		if (c != 0)
		{
			switch (c)
			{
				case '1':
					uart_puts(0,"Manual mode selected\r\n");
					modeFlags = 0b00000001;				
					break;
					
				case '2':
					uart_puts(0,"Test mode selected\r\n");
					modeFlags = 0b00000010;
					break;
					
				case '3':
					uart_puts(0,"Sample collection mode selected\r\n");
					modeFlags = 0b00000011;
					break;
					
				case '4':
					uart_puts(0,"Clean mode selected\r\n");
					modeFlags = 0b00000100;
					break;
					
				case '5':
					set_sample_volume(0, 10000);
					break;
					
				default:
					uart_puts(0,"Invalid operation mode\r\n");
					PCINT8flag = true;
					goto getinput;
					break;
			}
			oldChar = c;
			
			uart_puts(0,"\r\nDo you want to run after mode settings? (y/n)\r\n");
			c = USART0_getchar();
			if (c == 'y' || c == 'Y')
			{
				modeFlags |= 0b00001000;
				uart_puts(0,"Automatically run after mode settings\r\n");
				if (oldChar == '3')	// run sample mode
				{
					modeFlags |= 0b10000000;
				}
			}
			
			c = oldChar = 0;
		}
		
		
		

		// manual mode
		if ((modeFlags & 0b00000111) == 0b00000001){			// *** MANUAL MODE *** test 3LSB --> 001= manual mode
			#ifdef LCD_ENABLE
				lcd_clrscr();  // clear the lcd screen
				lcdGetModeAndUpdate(); //updates the LCD with the current mode
				lcdLine1Update("Enter terminal");
				lcdLine2Update("command");
			#endif
			#ifdef LOGGING_ENABLE
				LogInfo("Manual Mode, enter command:\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				uart_puts(0,"\r\nManual mode ON - Enter Command: ");
			#endif
			
			LedControl(LED_MANUAL_MODE,ON);
			//*** MANUAL MODE CODE ***
			while((modeFlags & 0b00000111) == 0b00000001) {				// test if manual modes flags are still set, will stop loop if isr (manual mode switch) toggles manual mode off
				//uart_puts(0,"uart command processor code loop\r\n");			//  ***TEST CODE comment out on deployment
				if (command_ready == TRUE) {
					//uart_puts(0,"manual mode found a command ready from ISR\r\n");		//  ***TEST CODE comment out on deployment
					copy_command ();// Here is where we will copy
					processCommand((void *)command_in);// and parse the command.
					command_ready = FALSE; // clear the command ready bit
				}
			}
			_delay_ms(100);
		}
		
		//test mode
		if ((modeFlags & 0b00000111) == 0b00000010){		// test 3LSB 010= test mode
			#ifdef LCD_ENABLE
				lcd_clrscr();
				lcdGetModeAndUpdate(); //updates the LCD with the current mode
				lcdLine1Update("Press RUN button to");
				lcdLine2Update("start");
			#endif
			LedAllOFF();
			LedControl(LED_TEST_MODE,1);   //  Turn on test mode LED
			
			
			#ifdef LOGGING_ENABLE
				LogInfo("Test mode on\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				printf("\r\ntest mode ON\r\n");
			#endif
			
			
			
			while ((modeFlags & 0b00000111) == 0b00000010){ // while modeFlags remain in test mode carry out this code which tests for the run button to be pressed
				if ((modeFlags & 0b00001111) == 0b00001010){	
					
					#ifdef LOGGING_ENABLE
						LogDebug("testing sequence run triggered\r\n");
						LogError("fram testing\r\n");
						LogData("fram testing\r\n");
					#endif
					#ifndef LOGGING_ENABLE
						printf("\r\ntest mode ON\r\n");
					#endif
				
 					
					 
				
					//fram_readAll();/* Read each byte of data on the FRAM into printf().*/
					//sampleTest();
					//while (1){						
// 					printf("in infinite while loop for testing valve driver\r\n");
// 					// valve function prototypes
// 					inlet_valve_denergise();
// 					_delay_ms(3000);
// 					inlet_valve_energise();
// 					_delay_ms(3000);
					
							
							
				// ********* mototr driver pin testing**************
// 					printf("in infinite while loop for testing motor\r\n");
// 					MOTORDRIVER_1_INPUT_1_PORT |= (1<<MOTORDRIVER_1_INPUT_1_PIN);
// 					MOTORDRIVER_1_INPUT_2_PORT |= (1<<MOTORDRIVER_1_INPUT_2_PIN);
// 					MOTORDRIVER_1_INPUT_PWM_PORT |= (1<<MOTORDRIVER_1_INPUT_PWM_PIN);
// 					MOTORDRIVER_1_INPUT_STBY_PORT |= (1<<MOTORDRIVER_1_INPUT_STBY_PIN);
// 					_delay_ms(1500);
// 					MOTORDRIVER_1_INPUT_1_PORT &= ~(1<<MOTORDRIVER_1_INPUT_1_PIN);
// 					MOTORDRIVER_1_INPUT_2_PORT &= ~(1<<MOTORDRIVER_1_INPUT_2_PIN);
// 					MOTORDRIVER_1_INPUT_PWM_PORT &= ~(1<<MOTORDRIVER_1_INPUT_PWM_PIN);
// 					MOTORDRIVER_1_INPUT_STBY_PORT &= ~(1<<MOTORDRIVER_1_INPUT_STBY_PIN);
// 					_delay_ms(1500);
							
							
//					*********** motor driver testing code***********
 				while (1)
 				{
				 BUOYANCYgoToSurface();
				 BUOYANCYgoToDepth(50);
 				}
				 

				 
				 
				


					//*********** motor driver pwm testing code***********
// 					printf("testing pwm");
// 					MotordriverRun(SAMPLE_PUMP,CLOCKWISE,255);// turn on preservative pump
// 					_delay_ms(3000);
// 					MotordriverRun(SAMPLE_PUMP,CLOCKWISE,100);// turn on preservative pump
// 					_delay_ms(3000);
					


// 					PRESSURE_SENSOR1_DDR |= (1<<PRESSURE_SENSOR1_PIN);
// 					PRESSURE_SENSOR1_PORT |= (1<<PRESSURE_SENSOR1_PIN); // set chip select to low to activate SPI
// 					_delay_ms(1000);
// 					PRESSURE_SENSOR1_PORT &= ~(1<<PRESSURE_SENSOR1_PIN); // set chip select to low to activate SPI
// 					_delay_ms(1000);	
						
							
// 					double bag_pressure = 0.00;
// 					printf("bag pressure default = %2.2f\r\n",bag_pressure);
// 					printf("cutoff pressure default = %2.2f\r\n",PRESSURE_CUTOFF_PSI);
// 					bag_pressure = ReadPressure(1);
// 					printf("pressure 1 = %2.2f of %2.2f\r\n",bag_pressure,PRESSURE_CUTOFF_PSI);
// 					_delay_ms(1000);
// 					bag_pressure = ReadPressure(2);
// 					printf("pressure 2 = %2.2f of %2.2f\r\n",bag_pressure,PRESSURE_CUTOFF_PSI);
// 					_delay_ms(1000);					
// 					bag_pressure = ReadPressure(3);
// 					printf("pressure 3 = %2.2f of %2.2f\r\n",bag_pressure,PRESSURE_CUTOFF_PSI);
// 					_delay_ms(1000);
//					}
				modeFlags &= ~(0b00001111);/* clear the modeflags after run button has been pressed and the code within the test mode has completed, returns to sleep mode)*/
				}
			}
		}
		
		// sample mode 
		if ((modeFlags & 0b00000111) == 0b00000011){		// test 3LSB 011
			#ifdef LCD_ENABLE
				lcd_clrscr();
				lcdGetModeAndUpdate(); //updates the LCD with the current mode
				lcdLine1Update("Press RUN button to");
				lcdLine2Update("start sampling");
			#endif
			LedAllOFF();
			LedControl(LED_SAMPLE_MODE,1);	//  Turn on sample mode LED
			
			
			#ifdef LOGGING_ENABLE
				LogInfo("sample collection mode\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				uart_puts(0, "sample collection mode ON\r\n");
			#endif
			
			
			
			
			while ((modeFlags & 0b00000111) == 0b00000011){
				if ((modeFlags & 0b10001111) == 0b10001011){ // will only collect sample is modeflags MSB is set, this indicates the device is ready to collect
					LogDebug("sample collection sequence triggered\r\n");
					//uart_puts(0, "sample collection sequence triggered\r\n");
					#ifdef LCD_ENABLE
						lcd_clrscr();
						lcdGetModeAndUpdate(); //updates the LCD with the current mode
						lcdLine1Update("Starting");
					#endif
					collectSample();	
					modeFlags &= ~(0b10001111);// clear the modeflags after interrupt back to default (sleep mode) ensuring that the ready to sample bit (MSB) is cleared to prevent another sample being taken
				}
				else if ((modeFlags & 0b10001111) == 0b00001011){ // modeflags MSB is not set, this indicates the device is has already sampled
					#ifdef LOGGING_ENABLE
						LogError("Sample collection Failed: Sample already taken, reset and try again\r\n");
					#endif
					#ifndef LOGGING_ENABLE
						printf("Sample collection sequence could not be triggered as sample has already been taken\r\n");
					#endif
					modeFlags &= ~(0b00001111);// clear the modeflags after interrupt back to default (sleep mode)
				}
			}
		}
		// cleaning mode 
		
		if ((modeFlags & 0b00000111) == 0b00000100){		// test 3LSB 100= cleaning mode
			#ifdef LCD_ENABLE
				lcd_clrscr();
				lcdGetModeAndUpdate(); //updates the LCD with the current   mode
				lcdLine1Update("Press RUN button to");
				lcdLine2Update("start cleaning run");
			#endif
			LedAllOFF();		
			#ifdef LOGGING_ENABLE
				LogInfo("clean mode ON\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				uart_puts(0,"\r\nclean mode ON - \r\n");
			#endif
			
			
			LedControl(LED_CLEANING_MODE,1);
			uint16_t elapsed_seconds = 0; // Make a new counter variable and initialize to zero
			while ((modeFlags & 0b00000111) == 0b00000100){  // while in cleaning mode
				if ((modeFlags & 0b00001111) == 0b00001100){  // if the run button is pressed		
					#ifdef LOGGING_ENABLE
						LogInfo("Clean mode started\r\n");
					#endif
					#ifndef LOGGING_ENABLE
					 printf("Clean mode started\r\n");
					#endif
					uint8_t wash_step = 0;
					for (wash_step = 1; wash_step <(NUMBER_OF_WASH_STEPS + 1) ; wash_step ++){
						switch(wash_step)
						{
							case 1  :
							printf("Wash step %i  \r\n", wash_step);
							break; /* optional */
							case 2  :
							printf("Wash step %i - Place inlet in   \r\n", wash_step);
							break; /* optional */
							case 3  :
							printf("Wash step %i  \r\n", wash_step);
							break; /* optional */
						}
						//Sample pump on
						inlet_valve_energise();  // open the 3 way inlet valve to the external environment to let sample/bleach water in
						MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,255);
						TCCR1B |= (1 << CS12) ; // Set up timer at Fcpu /256
						TCNT1 = 0; // Reset timer value
						elapsed_seconds = 0;
						
						while (elapsed_seconds < WASH_RUNTHROUGH_TIMEOUT_SECONDS){ // while elapsed time is below cutoff time.
							if ( TCNT1 >= 31249){  // if the counter has reached 1 second worth of ticks
								TCNT1 = 0; // Reset timer value
								elapsed_seconds ++;  // increment the seconds
								printf("elapsed_seconds %i of %i\r\n", elapsed_seconds, WASH_RUNTHROUGH_TIMEOUT_SECONDS);
							}
						}
						//Sample and preservative pumps off
						inlet_valve_denergise();
						MotordriverStop(SAMPLE_PUMP);
						printf("Wash stage %i complete\r\n", wash_step);
						modeFlags &= ~(0b00001000);// clear the run bit to await for next press to flush bleach out
						if (wash_step < NUMBER_OF_WASH_STEPS){
							
							while ((modeFlags & 0b00001000) == 0b00000000){  // while in run bit is clear							
							}
						}
					}
					modeFlags &= ~(0b00001111);// clear the modeflags after interrupt back to default (sleep mode)
				}
			}
						
		}
		

		// pre-run priming mode
		if ((modeFlags & 0b00000111) == 0b00000101){		// test 3LSB 100= cleaning mode
			#ifdef LCD_ENABLE
				lcd_clrscr();
				lcdGetModeAndUpdate(); //updates the LCD with the current mode
				lcdLine1Update("Press RUN button to");
				lcdLine2Update("start priming");
			#endif
			LedAllOFF();
			#ifdef LOGGING_ENABLE
				LogInfo("pre-run priming mode\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				printf("\r\nPre-run priming mode ON");
			#endif
			LedControl(LED_PRIMING_MODE,1);   //  Turn on test mode LED
			while ((modeFlags & 0b00000111) == 0b00000101){
				if ((modeFlags & 0b00001111) == 0b00001101){
					LogInfo("pre run priming mode started\r\n");
					//printf("pre run priming mode started");
					// insert cleaning code here
					int a=0;
					for(a = 1; a <= 50; a ++){
						printf("iteration %i/20 of for loop...\r\n",a);
						//PH6_pwm_set(0);
						//PB7_pwm_set(0);
						printf("ph6 pwm low...\r\n");
						_delay_ms(2000);
						//PH6_pwm_set(255);
						//PB7_pwm_set(255);
						printf("ph6 pwm high...\r\n");
						_delay_ms(2000);
					}
					modeFlags &= ~(0b00001111);// clear the modeflags after interrupt back to default (sleep mode)
				}
			}	
		}
		
		
				// FRAM Erase mode
				if ((modeFlags & 0b00000111) == 0b00000110){		// test 3LSB 100= cleaning mode
					#ifdef LCD_ENABLE
					lcd_clrscr();
					lcdGetModeAndUpdate(); //updates the LCD with the current mode
					lcdLine1Update("Press RUN button erase FRAM");
					lcdLine2Update("start priming");
					#endif
					LedAllOFF();
					#ifdef LOGGING_ENABLE
						LogInfo("FRAM mode\r\n");
					#endif
					#ifndef LOGGING_ENABLE
						printf("%s","Press run button to erase FRAM\r\n");
					#endif
					LedControl(LED_PRIMING_MODE,1);   //  Turn on test mode LED
					while ((modeFlags & 0b00000111) == 0b00000110){ //while in fram erase mode
						if ((modeFlags & 0b00001111) == 0b00001110){ //check for run button press
							#ifdef LOGGING_ENABLE
								LogDebug("fram ERASE selected \r\n");
							#endif
							#ifndef LOGGING_ENABLE
								printf("fram ERASE selected\r\n");
							#endif
							fram_resetWriteAddress();
							modeFlags &= ~(0b00001111);// clear the modeflags after interrupt back to default (sleep mode)
						}
					}
				}

		
		//UART input mode mode
		if ((modeFlags & 0b00000111) == 0b00000111){		// test 3LSB 110 = UART input mode
			#ifdef LCD_ENABLE
			lcd_clrscr();
			lcdGetModeAndUpdate(); //updates the LCD with the current mode
			lcdLine1Update("Enter text 1");
			lcdLine2Update("text 2");
			#endif
			LedAllOFF();
			#ifdef LOGGING_ENABLE
				LogInfo("UART input mode ON\r\n");
			#endif
			#ifndef LOGGING_ENABLE
				printf("UART input mode ON\r\n");
			#endif
			while ((modeFlags & 0b00000111) == 0b00000111){ //while in UART mode
				if ((modeFlags & 0b00001111) == 0b00001111){ // when run button pressed
					// replace For loop by adding tasks here....
					printf("Exit UART input mode\r\n");
					for (char i1=0; i1<10; i1++){
						// Exit UART input mode in 10 seconds
						_delay_ms(10);
					}
					modeFlags &= ~(0b00001111);// clear the modeflags after interrupt back to default (sleep mode)
					if (usart1Stop){
						// Before AVR going back to sleep, disable UART1 and trigger INT2
						USART1_disable();		// Disable UART1 and trigger INT2
						usart1Stop = 0;			// Clear uart1 stop flag
					}
				}
			}
		}
		else{
			#ifdef LOGGING_ENABLE
				LogInfo("no valid operation mode selected\r\n");
			#else
				printf("no valid operation mode selected");
			#endif
		}

	}
	return(0);
}





// ********************************************************************************
// Function definitions
// ********************************************************************************



   
void init_interrupts(void) {
	#ifdef LOGGING_ENABLE
		LogDebug("init_interrupts() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("init_interrupts() started\r\n");
	#endif
	
	EIMSK &= ~(1 << INT7);								// External Interrupt Mask Register - Clear INT7 enable bit before changing the ISC (interrupt sens control in the EICRA below)
	//EIMSK &= ~(1 << INT6);								// External Interrupt Mask Register - Clear INT6 enable bit before changing the ISC (interrupt sens control in the EICRA below)
	EIMSK &= ~(1 << INT5);								// External Interrupt Mask Register - Clear INT5 enable bit before changing the ISC (interrupt sens control in the EICRA below)
	EIMSK &= ~(1 << INT4);								// External Interrupt Mask Register - Clear INT5 enable bit before changing the ISC (interrupt sens control in the EICRA below)
	//EIMSK &= ~(1 << INT2);								// External Interrupt Mask Register - Clear INT2 enable bit before changing the ISC (interrupt sens control in the EICRA below)
	
	EIFR |= (1<< INTF7);								// clear any interrupts waiting on INT7
	//EIFR |= (1<< INTF6);								// clear any interrupts waiting on INT6
	EIFR |= (1<< INTF5);								// clear any interrupts waiting on INT5
	EIFR |= (1<< INTF4);								// clear any interrupts waiting on INT5
	//EIFR |= (1<< INTF2);								// clear any interrupts waiting on INT2
	
	EIMSK |= (1 << INT7);								//External Interrupt Mask Register - turn on interrupts on INT7 after setting the ISC (turned off above to prevent ISR during init)
	//EIMSK |= (1 << INT6);								//External Interrupt Mask Register - turn on interrupts on INT6 after setting the ISC (turned off above to prevent ISR during init)
	EIMSK |= (1 << INT5);								//External Interrupt Mask Register - turn on interrupts on INT5 after setting the ISC (turned off above to prevent ISR during init)
	EIMSK |= (1 << INT4);								//External Interrupt Mask Register - turn on interrupts on INT5 after setting the ISC (turned off above to prevent ISR during init)

	//EIMSK |= (1 << INT2);								//External Interrupt Mask Register - turn on interrupts on INT2 after setting the ISC (turned off above to prevent ISR during init)
	//sei();												// set (global) interrupt enable bit

	#ifdef LOGGING_ENABLE
		LogDebug("init_interrupts() closed\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("init_interrupts() closed\r\n");
	#endif
}											// initializes





// *** Serial command functions

void copy_command (void) {
	// The UART might interrupt this - don't let that happen!
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		//printf("copy_command: data");
		// Copy the contents of data_in into command_in, memcpy always copies a set number of bytes.
		memcpy((void *)command_in, (void *)data_in, 16);
		// Now clear data_in, the UART can reuse it now
		memset((void *)data_in, 0, 16);
		//uart_puts(0,"Command coped from data_in to command_in \r\n");			//  ***TEST CODE comment out on deployment
		//printf("copy_command: command_in = %s", command_in);
	}
}


/*
unsigned long parse_assignment (){
	char *pch; //
	char cmdValue[16];
	// Find the position the equals sign is
	// in the string, keep a pointer to it
	pch = strchr(command_in, '=');
	// Copy everything after that point into
	// the buffer variable
	strcpy(cmdValue, pch+1);
	// Now turn this value into an integer and
	// return it to the caller.
	return atoi(cmdValue);
}
*/

void processCommand(char command_string[]){
	// takes the first 3 chars and finds corresponding command from command_structure_array,
	//calls the corresponding function and passes it the string for the subsequent chars from the command
	// divide the command string into a command_name (first 3 chars) and a command_value_string (all subsequent chars until string end)
	//printf("processCommand: command_string = %s\r\n", command_string);//prints the input strig
	
	//remove \n and \r which have been tacked on to the string from source
	int i;	
	for (i=0  ;  i < (strlen(command_string))  ;  i++){
		if ((command_string[i] == '\n')	||	(command_string[i] == '\r')) {
			command_string[i] = '\0';
		}
	}
	//printf("command_string after \\n removal is %s\r\n",command_string);
	int command_string_length;          // holds the length of the string
	command_string_length = strlen(command_string); //calculates the string length
	//printf("command_string_length after \\n removal is %d\r\n",command_string_length);
	// get command_name from command_string
	/*char command_name[4];*/
	strncpy(command_name,command_string,3);
	command_name[3] = 0x00; // add null character to terminate string in 4th place
	//printf("command_name = %s\r\n", command_name);

	// get command_value and remove the command name from the from (remove first 3 char)
	char command_value_string[(command_string_length -3)];  // a string for the command value to go into
	for (i=3; i< command_string_length; i++){          // this takes the command_string elements after the first 3 and puts them into a new array(string)
		command_value_string[i-3] = command_string[i];
	}
	command_value_string[command_string_length-3] = 0x00; // add null character to terminate new string
	//printf("command_value_string = %s\n",command_value_string);


//process the actual command string
	//printf("command_name = %s\n", command_name);
	int command_iteration;
	for (command_iteration = 0; command_iteration < NUM_OF_COMMANDS_IN_ARRAY; command_iteration++){
		//printf("command_iteration %d\n",command_iteration);
		//printf("command_name = %s\n", command_name);
		//printf("command in array position =%s\n", command_structure_array[command_iteration].name);
		
		// 	compare the input command with the commands held in progmem
		if ((strcmp (command_name,(const char*)(pgm_read_word(&(command_structure_array[command_iteration].name)))    )) == 0){        //****** added progmemcode
			
			printf("command found <%s>!!!!\n",command_name);
			// **** eneter execution code here*****
			void (*commandFunctionpointer)(char[]); // declares a function pointer that will take the command from the command array structure
			printf ("command_value_string passed (to be sent by function pointer) to function pointer = %s\n",command_value_string);
			commandFunctionpointer = (void(*)(char[]))(pgm_read_word(&(command_structure_array[command_iteration].execute)));     //******** added progmemcode
				
			
			(*commandFunctionpointer)(command_value_string);
			printf ("command_value_string_passed = %s and is %d long", command_value_string, strlen(command_value_string) );
			break;
		}
		else if (command_iteration == (NUM_OF_COMMANDS_IN_ARRAY -1)) // if the command has not been found by the end of the command structure array then print that no command has been found
		{
			printf("no valid command found for the command entered <%s>\n",command_name);
		}
	}
	command_string = "000000000000000";  // reset the command string to stop previous commands being carried over
}


void commandTest(char command_str[]){
	// convert the received command_str to an int
	printf("commandTest function passed the string = %s\n",command_str);
	int command_value = atoi(command_str);
	printf("command value as int = %d\n",command_value);

}


/* --- STAN functions --- */

int collectSample(void){
	//---------------------------------------------------------
	// FUNCTION NAME: collectSample
	// ARGUMENTS: none
	// RETURNS: int
	// DESCRIPTION: this function performs the sampling process for the STAN device. COLLECT SAMPLE --> INLET LINE WASH --> PRESERVATIVE TO FILTER -->
	//---------------------------------------------------------
	
	#ifdef LOGGING_ENABLE
	LogDebug("collectSample() function entered\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	uart_puts(0,"collectSample() function entered\r\n");
	#endif
	uint16_t elapsed_seconds = 0; // Make a new counter variable and initialize to zero
	double bag_pressure = 0.00;
	double filter_pressure = 0.00;
	uint8_t sample_pump_pwm_speed = SAMPLE_PUMP_STARTING_PWM_SPEED;
	modeFlags |= 0b00000001;// set modeflags to sampling mode on
	//Sample pump on
	TCCR1B |= (1 << CS12) ; // Set up timer at Fcpu /256
	inlet_valve_energise();  // open the 3 way inlet valve to the external environment to let sample water in
	MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,sample_pump_pwm_speed);// turn on sample motor to start pushing sample through sterivex filter'
	#ifdef LOGGING_ENABLE
	LogInfo("Water sampling - STARTED\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	uart_puts(0,"Water sampling - STARTED\r\n");
	#endif
	// log the parameters of the sampling
	char samplingparameterssstr[120];

	sprintf(samplingparameterssstr,"Timeout:%is, BagP cutoff:%2.2fpsi, FilterPmax:%2.2fpsi\r\n",SAMPLE_PUMP_TIMEOUT_SECONDS,BAG_PRESSURE_CUTOFF_PSI, MAX_FILTER_PRESSURE_PSI); // this is for plotting the log data
	#ifdef LOGGING_ENABLE
	LogData(samplingparameterssstr);
	LogData("CSVheadings[,T (sec),Bag P (psi), Filter P (psi), Pump speed (pwm) ]\r\n");
	#else
	printf("sampling parameters\r\n");
	#endif
		
	
	// while the  modeflags are set for sample collection, && pressure is below cuttoff && time is below cutoff time.
	while (
	(elapsed_seconds < SAMPLE_PUMP_TIMEOUT_SECONDS) &&
	(bag_pressure < BAG_PRESSURE_CUTOFF_PSI) &&
	((elapsed_seconds < MIN_SAMPLE_TIME_SECONDS) || (filter_pressure < MAX_FILTER_PRESSURE_PSI)) &&
	(sample_pump_pwm_speed > MINIMUM_SAMPLE_PUMP_PWM_SPEED) &&
	(v.volume_mean < settings.volumeSample) 
	){ // run until pressure sensor tells us the bag is full
		if ( TCNT1 >= 31249){  // if the counter has reached 1 second worth of ticks
			TCNT1 = 0; // Reset timer value
			elapsed_seconds ++;  // increment the seconds
			v.i2++;				// increment no. of seconds offset (for flow meter)
			
			#ifdef LCD_ENABLE
			lcd_gotoxy(13,0); // move to the time characters to clear them
			lcd_puts("      ");  // clear the time characters with spaces
			lcdUpdateSamplingTime(elapsed_seconds);
			lcdLine1Update("Pumping    mPWM ");
			lcdUpdatePWMValue(sample_pump_pwm_speed);
			#endif
			//printf("about to check bag pressure\n");
			_delay_ms(3); //to give pressure sensor time to retest, if this is not here some values will come back as zero
			bag_pressure = ReadPressure(1);
			filter_pressure = ReadPressure(2);
			#ifdef LCD_ENABLE
			lcdUpdatePressureInfo(filter_pressure,bag_pressure);
			#endif
			
			if (filter_pressure > (MAX_FILTER_PRESSURE_PSI- 5)){ // if the filter pressure comes within 10psi of the maximum
				sample_pump_pwm_speed -= 10; // reduce the sample pumping speed variable
				MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,sample_pump_pwm_speed); // reduce the speed of the motor
			}
			
			
			
			if ((elapsed_seconds % DATA_RECORD_INTERVAL) == 0) // record data to printf and FRAM every #define DATA_RECORD_INTERVAL seconds
			{
				char datastr[120];
				// TODO only need to log counter_11. counter12 and counter_13 are included here for test purposes...
				//sprintf(datastr,"CSV[,%i, %2.2f,%2.2f,%i,%u,%lu,%lu,%lu,]\r\n", elapsed_seconds,bag_pressure,filter_pressure, sample_pump_pwm_speed,flow_vol_mL,counter_11.pcint_total,counter_12.pcint_total,counter_13.pcint_total); // this is for plotting the log data
				sprintf(datastr,"CSV[,%i,%2.2f,%2.2f,%i,%u,%.2f]\r\n", elapsed_seconds,bag_pressure,filter_pressure, sample_pump_pwm_speed,v.flr_mean,v.volume_mean);
				#ifdef LOGGING_ENABLE
				LogData(datastr);
				#endif
				#ifndef LOGGING_ENABLE
				printf("%s",datastr);
				#endif
				
				// fram logging code
				//char logging_str[40] = "the quick brown fox jumped over the lazy dog\n";
				//sprintf(logging_str,"%i,%2.2f,%2.2f,%i\r\n", elapsed_seconds,bag_pressure,filter_pressure, sample_pump_pwm_speed); // this is for plotting the log data
				//fram_writeString("the quick brown fox jumped over the lazy dog\n");// Write a sequential string of data to next FRAM location
				//fram_readAll();// Read each byte of data on the FRAM into printf(). // this is here for testing
				
				// terminal printing code
				//printf("elapsed_seconds %i of %i\r\n", elapsed_seconds, SAMPLE_PUMP_TIMEOUT_SECONDS);
				//printf("Sample collection bag pressure = %2.2f of %2.2f\r\n",bag_pressure,BAG_PRESSURE_CUTOFF_PSI);
				//printf("Filter pressure = %2.2f of cutoff %2.2f \r\n",filter_pressure, MAX_FILTER_PRESSURE_PSI);
				
			}
			
			
		}
		
		// ---- Flow meter code ----
		v = volume_instants(v);
		v = volume_means(v);
		// ---- End flow meter code ----
	}
	
	printf("Sample volume collected = %.2f ml\r\n", v.volume_mean);
	//pump off
	MotordriverStop(SAMPLE_PUMP);
	// switch valve off, this opens pump to preservative
	inlet_valve_denergise(); // switch the inlet 3 way valve back to deengergised mode, open to preservative inlet
	
	#ifdef LOGGING_ENABLE
	LogInfo("Water sampling - COMPLETE\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	uart_puts(0,"Water sampling - COMPLETE\r\n");
	#endif
	
	//pump preservative to inlet line to suppress growth
	//uart_puts(0,"Inlet line wash - STARTED\r\n");
	//uart_puts(0,"Inlet line wash - COMPLETE\r\n");
	
	// pump Preservative over the filter
	MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,sample_pump_pwm_speed);// turn on sample motor
	uint16_t preservation_elapsed_seconds = 0; // reset the pump timer to zero
	TCNT1 = 0; // Reset timer value
	while (preservation_elapsed_seconds < PRESERVATIVE_PUMP_TIMEOUT_SECONDS){
		if ( TCNT1 >= 31249){  // if the counter has reached 1 second worth of ticks
			TCNT1 = 0; // Reset timer value
			preservation_elapsed_seconds ++;  // increment the seconds
			printf("elapsed_seconds of preservative %i\r\n", preservation_elapsed_seconds);
			
			#ifdef LCD_ENABLE
			lcdUpdateSamplingTime(preservation_elapsed_seconds);
			lcdLine1Update("Preserving mPWM ");
			lcdUpdatePWMValue(sample_pump_pwm_speed);
			#endif
			
		}
	}
	//printf("Preservative pumping took %i\r\n", preservation_elapsed_seconds);
	//preservative pump off
	MotordriverStop(SAMPLE_PUMP); // turn off sample pump (pump1)
	

	modeFlags &= ~(0b00000001);// set modeflags to sampling mode off
	
	#ifdef LOGGING_ENABLE
	LogInfo("collect sample() function completed\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	uart_puts(0,"collect sample() function completed\r\n");
	#endif
	#ifdef LCD_ENABLE
	lcdLine1Update("SAMPLING");
	#endif
	return 0;
}



/* //TODO is this used? 
int cleaningRun(void)
{
	MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,255); // reduce the speed of the motor
}*/


int sampleTest(void){
	#ifdef LOGGING_ENABLE
		LogDebug("sampleTest function entered\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		uart_puts(0,"sampleTest function entered\r\n");
	#endif
	
	/*
	double bag_pressure = 9.99;
	int i;	
	for (i=0;i<3;i++)
	{
	bag_pressure = ReadPressure(1);
	printf("Sample collection bag pressure = %2.2f of %2.2f\r\n",bag_pressure,PRESSURE_CUTOFF_PSI);
	}
	*/
	
	// test LEDs
	//uart_puts(0,"Testing LEDs\r\n");
// 	int i;
// 	for (i=0;i<10; i++){
// 		LED_PORT |= (1 << SAMPLE_MODE_LED_PIN);// set sampling led high
// 		_delay_ms(50);
// 		LED_PORT &=~(1 << SAMPLE_MODE_LED_PIN);// set sampling led low
// 		_delay_ms(100);
// 	}
// 	for (i=0;i<10; i++){
// 		LED_PORT |= (1 << 3);// set  led1 high
// 		_delay_ms(50);
// 		LED_PORT &=~(1 << 3);// set  led1 low
// 		_delay_ms(100);
// 	}
// 	for (i=0;i<10; i++){
// 		LED_PORT |= (1 << 3);// set led2 high
// 		_delay_ms(50);
// 		LED_PORT &=~(1 << 3);// set  led2 low
// 		_delay_ms(100);
// 	}


	// //test input valve
	uart_puts(0,"Testing input valve\r\n");
	int i;
	for (i=0; i<3;i++)
	{
		inlet_valve_denergise();
		_delay_ms(100);
		printf("inlet_valve_denergise() called\r\n");
		inlet_valve_energise();
		printf("inlet_valve_energise() called\r\n");
		_delay_ms(100);
	}
	
	//tests pump
	uart_puts(0,"Testing pump\r\n");
	MotordriverRun(SAMPLE_PUMP,COUNTER_CLOCKWISE,255);// turn on sample motor
	_delay_ms(1500); // 2 second run
	MotordriverStop(SAMPLE_PUMP); /* turn off sample pump (pump1) */
	_delay_ms(1000); // 2 second run
	MotordriverRun(SAMPLE_PUMP,CLOCKWISE,255);// turn on sample motor
	_delay_ms(1500); // 2 second run
	MotordriverStop(SAMPLE_PUMP); /* turn off sample pump (pump1) */
	
	double bag_pressure = 0.00;
	double filter_pressure = 0.00;
	bag_pressure = ReadPressure(1);
	filter_pressure = ReadPressure(2);
	printf("Sample collection bag pressure = %2.2f of %2.2f\r\n",bag_pressure,BAG_PRESSURE_CUTOFF_PSI);
	printf("Filter pressure = %2.2f of cutoff %2.2f \r\n",filter_pressure, MAX_FILTER_PRESSURE_PSI);
	
	
	
	// set sampling led low
	LedControl(LED_SAMPLE_MODE,0);
	uart_puts(0,"Testing complete\r\n");
	return 0;
	//test RTC
}




void init_push_buttons(void){
	#ifdef LOGGING_ENABLE
		LogDebug("init_push_buttons() started\r\n"); 
	#endif
	#ifndef LOGGING_ENABLE
		printf("init_push_buttons() started\r\n");
	#endif
	PUSH_BUTTON_DDR |= (1<<PUSH_BUTTON_0_PIN)|(1<<PUSH_BUTTON_1_PIN)|(1<<PUSH_BUTTON_2_PIN);	// set internal pull up for PE4, PE5, are the push button inputs from the schmitt trigger
	#ifdef LOGGING_ENABLE
		LogDebug("init_push_buttons() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("init_push_buttons() complete\r\n");
	#endif
	
}



/*		MAX 3231 RTC Functions	*/
/*****************************************************/

void testsetClockfromstring(char command_value_string_passed[]){
	printf ("command_value_string_passed = %s", command_value_string_passed);
}




/*		PWM Functions	*/
/*****************************************************/

void pwm_init_PL5(){
	#ifdef LOGGING_ENABLE
	LogDebug("pwm_init_PL5 started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("pwm_init_PL5 started\r\n");
	#endif
	//pwm on pin PL5 / OC5C
	// make sure to make OC5C pin (pin PL5 for atmega2560) as output pin
	DDRL |= (1<<PL5);
	// set the waveform generation mode to Fast PWM, 8-bit , Update of 	OCR5x at bottom, TOV5 flag set on top
	TCCR5A  |= (1 << WGM50);
	TCCR5B  |= (1 << WGM52);
	// set the mode
	TCCR5A |= (1<<COM5C1);   // Clear OC5C on Compare Match
	// set no prescaler
	TCCR5B  |= (1 << CS50);
	#ifdef LOGGING_ENABLE
		LogDebug("pwm_init_PL5 started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("pwm_init_PL5 started\r\n");
	#endif
}

void pwm_init_PL3(){
	#ifdef LOGGING_ENABLE
		LogDebug("pwm_init_PL3 started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("pwm_init_PL3 started\r\n");
	#endif
	//pwm on pin PL3 / OC5A
	// make sure to make OC5A pin (pin PL3 for atmega2560) as output pin
	DDRL |= (1<<PL3);
	// set the waveform generation mode to Fast PWM, 8-bit , Update of 	OCR5x at bottom, TOV5 flag set on top
	TCCR5A  |= (1 << WGM50);  //
	TCCR5B  |= (1 << WGM52);	//
	// set the mode
	TCCR5A |= (1<<COM5A1);   // Clear OC5A on Compare Match
	// set no prescaler
	TCCR5B  |= (1 << CS50);
	#ifdef LOGGING_ENABLE
		LogDebug("pwm_init_PL3 started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("pwm_init_PL3 started\r\n");
	#endif
}

// void PB7_pwm_set(int pwm_value){
// 	//pwm on Pb7
// 	// make sure to make OC0A0 pin (pin PB7 for atmega2560) as output pin
// 	DDRB |= (1<<PB7);
// 	// initialize TCCR0A: these bit set to non inverting mode .
// 	TCCR0A |= (1<<COM0A1);
// 	// initialize TCCR0A: these bit determine the PWM mode
// 	TCCR0A |= (1<<WGM00)|(1<<WGM01);
// 	// initialize TCCR0A: these bit determine the clock prescaler (setting CS00 makes no prescaler)
// 	TCCR0B |= 1<<CS00;
// 	/* Set initial PWM value to zero*/
// 	OCR0A = 0;
// }

// void PH6_pwm_set(int pwm_value){
// 	//pwm on pin PH6 for OCR2B
// 	// make sure to make OC0A0 pin (pin PH6 for atmega2560) as output pin
// 	DDRH |= (1<<PH6);
// 	// initialize TCCR2B: these bit set to non inverting mode .
// 	TCCR2A |= (1<<COM2B1);
// 	// initialize TCCR0A: these bit determine the PWM mode
// 	TCCR2A |= (1<<WGM20)|(1<<WGM21); //fast pwm mode
// 	// initialize TCCR2A: these bit determine the clock prescaler (setting CS00 makes no prescaler)
// 	TCCR2B |= 1<<CS20;
// 	/* Set initial PWM value to zero*/
// 	OCR2B = 0;
// 
// 	// set PWM for 50% duty cycle
// 	//OCR2B = 128;
// }

// void PH6_pwm_set(uint8_t pwm_value){
// 	//set pwm pin PH6/OCR2B
// 	OCR2B = pwm_value;
// }

// void PB7_pwm_set(uint8_t pwm_value){
// 	//set pwm pin PB7/OCR0A
// 	OCR0A = pwm_value;


void PL5_pwm_set(int pwm_value){
	//set pwm pin PL5
	OCR5C= pwm_value; // assugn an 8-bit value to the pwm register
}

void PL3_pwm_set(int pwm_value){
	//set pwm pin PL3
	OCR5A= pwm_value; // assugn an 8-bit value to the pwm register
}









/*		Sleep Mode Functions	*/
/*****************************************************/

void init_sleep_mode(void)
{
	#ifdef LOGGING_ENABLE
	LogDebug("init_sleep_mode()started\r\n");
	#endif
	SMCR |= (1<<SM1);	// this sets the sleep mode control register to select power down mode
	#ifdef LOGGING_ENABLE
	LogDebug("init_sleep_mode() complete\r\n");
	#endif
}

void sleep_mode_on(void)
{
	#ifdef LOGGING_ENABLE
		LogInfo("Entering sleep mode\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		uart_puts(0,"entering sleep mode\r\n");
	#endif
	_delay_ms(2); // give printf above time to send
	usart1Stop = 1; // set this flag to disable UART1 and enable wakeup pin INT2 
	PCMSK1 |= (1 << PCINT8);		// Enable individual interrupt PCINT8 wakeup from PC (RX0)
	
	SMCR |= (1<<SE);  // sleep mode enable bit
	sleep_cpu();     
	SMCR &= ~(1<<SE);  // sleep mode enable bit off
	//uart_puts(0,"exiting sleep mode\r\n");
	#ifdef LOGGING_ENABLE
		LogDebug("Exiting sleep mode\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("Exiting sleep mode\r\n");
	#endif
}


/*		LED Control Functions	*/
/*****************************************************/

//************** LED blinking code for debugging************

void Ledinit(void)
{
	#ifdef LOGGING_ENABLE
		LogDebug("Ledinit() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("Ledinit() started\r\n");
	#endif
	LED_0_DDR |= (1<<LED_0_PIN);
	LED_1_DDR |= (1<<LED_1_PIN);
	LED_2_DDR |= (1<<LED_2_PIN);
	LED_3_DDR |= (1<<LED_3_PIN);
	LED_4_DDR |= (1<<LED_4_PIN);
	LED_5_DDR |= (1<<LED_5_PIN);
	LED_6_DDR |= (1<<LED_6_PIN);
	LED_7_DDR |= (1<<LED_7_PIN);
	#ifdef LOGGING_ENABLE
		LogDebug("Ledinit() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("Ledinit() complete\r\n");
	#endif
}

void LedBlink(uint8_t blink_duration_in_seconds)
{
 uint8_t j;
 uint8_t i=0;
 j = blink_duration_in_seconds * 4;
	 for (i=0; i<j; i++) {
		LED_0_PORT |= (1<<LED_0_PIN);//  Turn on LEDS to show activity
		LED_1_PORT |= (1<<LED_1_PIN);
		LED_2_PORT |= (1<<LED_2_PIN);
		LED_3_PORT |= (1<<LED_3_PIN);
		LED_4_PORT |= (1<<LED_4_PIN);
		LED_5_PORT |= (1<<LED_5_PIN);
		LED_6_PORT |= (1<<LED_6_PIN);
		LED_7_PORT |= (1<<LED_7_PIN);
		_delay_ms(30);					
		LED_0_PORT &= ~(1<<LED_0_PIN);//  Turn off LEDS to show activity
		LED_1_PORT &= ~(1<<LED_1_PIN);
		LED_2_PORT &= ~(1<<LED_2_PIN);
		LED_3_PORT &= ~(1<<LED_3_PIN);
		LED_4_PORT &= ~(1<<LED_4_PIN);
		LED_5_PORT &= ~(1<<LED_5_PIN);
		LED_6_PORT &= ~(1<<LED_6_PIN);
		LED_7_PORT &= ~(1<<LED_7_PIN);
		_delay_ms(30);		//debug
	 }
}

void LedControl(uint8_t led_number, uint8_t on_off){		//function to control manual mode led toggling
	if (on_off == 1) {
		switch(led_number) {
			case 0  :
				LED_0_PORT |= (1<<LED_0_PIN);
				break; 
			case 1  :
				LED_1_PORT |= (1<<LED_1_PIN);
				break; 
			case 2  :
				LED_2_PORT |= (1<<LED_2_PIN);
				break; 
			case 3  :
				LED_3_PORT |= (1<<LED_3_PIN);
				break;
			case 4  :
				LED_4_PORT |= (1<<LED_4_PIN);
				break; 
			case 5  :
				LED_5_PORT |= (1<<LED_5_PIN);
				break;
			case 6  :
				LED_6_PORT |= (1<<LED_6_PIN);
				break;
			case 7  :
				LED_7_PORT |= (1<<LED_7_PIN);
				break;
			default :
			printf("no valid led_number found to turn on in LedControl \r\n");
		}
	}else if(on_off == 0) {
		switch(led_number) {
			case 0  :
				LED_0_PORT &= ~(1<<LED_0_PIN);
				break;
			case 1  :
				LED_1_PORT &= ~(1<<LED_1_PIN);
				break;
			case 2  :
				LED_2_PORT &= ~(1<<LED_2_PIN);
				break;
			case 3  :
				LED_3_PORT &= ~(1<<LED_3_PIN);
				break;
			case 4  :
				LED_4_PORT &= ~(1<<LED_4_PIN);
				break;
			case 5  :
				LED_5_PORT &= ~(1<<LED_5_PIN);
				break;
			case 6  :
				LED_6_PORT &= ~(1<<LED_6_PIN);
				break;
			case 7  :
				LED_7_PORT &= ~(1<<LED_7_PIN);
				break;
			default :
			printf("no valid led_number found to turn off LedControl \r\n");
		}
	}else {
		// error condition: incorrect arguments to manualModeled()
		printf("incorrect arguments sent to manualModeled() \r\n");
	}
}


void LedAllOFF(void)
{
	LED_0_PORT &= ~(1<<LED_0_PIN);
	LED_1_PORT &= ~(1<<LED_1_PIN);
	LED_2_PORT &= ~(1<<LED_2_PIN);
	LED_3_PORT &= ~(1<<LED_3_PIN);
	LED_4_PORT &= ~(1<<LED_4_PIN);
	LED_5_PORT &= ~(1<<LED_5_PIN);
	LED_6_PORT &= ~(1<<LED_6_PIN);
	LED_7_PORT &= ~(1<<LED_7_PIN);
}
	
	/*		LCD Control Functions	*/
	/*************************************************************************
	Using Peter Fleury <pfleury@gmx.ch>  http://tinyurl.com/peterfleury lcd.h and lcd.c files
	and 20x4 character 3v3 LCD display from eBay
	Line 1 is mode line
	Line 2 is status
	Line 3 is information/prompt line
	Line 4 is debug/data line
	*************************************************************************/
	
void lcdPrintAllLines(const char line0[],const char line1[],const char line2[],const char line3[])
{
	lcd_clrscr(); //clear the screen
	lcd_gotoxy(0,0);/* move cursor to position 0 on line 0 (zero indexed)*/ 
	lcd_puts(line0);/* put string to display (line 0) with linefeed */
	lcd_gotoxy(0,1);/* move cursor to position 0 on line 1 (zero indexed)*/
	lcd_puts(line1);/* put string to display (line 1) with linefeed */
	lcd_gotoxy(0,2);/* move cursor to position 0 on line 2 (zero indexed)*/
	lcd_puts(line2);/* put string to display (line 1) with linefeed */
	lcd_gotoxy(0,3);/* move cursor to position 0 on line 3 (zero indexed)*/ 
	lcd_puts(line3);/* put string to display (line 1) with linefeed */
}

void lcdGetModeAndUpdate()  // updates the top lcd line with the current mode
{
	lcdClearLine(0);
	// get the current mode from modeflags variable
	uint8_t current_stan_mode = (modeFlags & 0b00000111); // test 3LSB of modeflages gives us the current mode
	
	switch(current_stan_mode) {

		case 0b00000000  :   //sleep mode
		lcdLine0Update("Sleep Mode:");
		break; /* optional */
		
		case 0b00000001  :	//manunal mode
		lcdLine0Update("Manual Mode:");
		break; /* optional */
		
		case 0b00000010  :	// test mode
		lcdLine0Update("Test Mode:");
		break; /* optional */
		
		case 0b00000011  :		//sample mode
		lcdLine0Update("Sample Mode:");
		break; /* optional */
		
		case 0b00000100  : //Clean mode
		lcdLine0Update("Cleaning Mode:");
		break; /* optional */
		
		case 0b00000101  : //priming mode
		lcdLine0Update("Priming Mode:");
		break; /* optional */
		
		case 0b00000110  :
		lcdLine0Update("UART input mode");
		break; /* optional */
		
		case 0b00000111  :
		lcdLine0Update("FRAM ERASE mode:");
		break; /* optional */
		
		/* you can have any number of case statements */
		default : /* Optional */
		lcdStatusLineUpdate("invalid mode");
	}
	lcdShowIfRunCompleted();

}



	
void lcdStatusLineUpdate(const char statusText[])  // clears and adds text to lcd line 1 (second line down)
{
	lcdClearLine(1);
	lcd_gotoxy(7,1);/* move cursor to position 8 on line 1 (zero indexed), this is the text after the "status:" text */
	lcd_puts("Status:");/* put string to display (line 1) with linefeed */
	lcd_gotoxy(7,1);/* move cursor to position 8 on line 1 (zero indexed), this is the text after the "status:" text */
	lcd_puts(statusText);/* put string to display (line 1) with linefeed */
}

void lcdLine0Update(const char line0text[20])  // clears and adds text to lcd line 1 (second line down)
{
	lcdClearLine(0);
	lcd_home();
	lcd_puts(line0text);/* put string to display (line 1) with linefeed */
}
	
void lcdLine1Update(const char line1text[20])  // clears and adds text to lcd line 1 (second line down)
{
	lcdClearLine(1);
	lcd_gotoxy(0,1);/* move cursor to position 0 on line 1 (zero indexed) */
	lcd_puts(line1text);/* put string to display (line 1) with linefeed */
}	
	
void lcdLine2Update(const char line2text[20])
{
	lcdClearLine(2);
	lcd_gotoxy(0,2);/* move cursor to position 0 on line 1 (zero indexed) */
	lcd_puts(line2text);/* put string to display (line 1) with linefeed */
}
	
void lcdLine3Update(const char line3text[20])
{
	lcdClearLine(3);
	lcd_gotoxy(0,3);/* move cursor to position 0 on line 1 (zero indexed) */
	lcd_puts(line3text);/* put string to display (line 1) with linefeed */
}

void lcdClearLine(uint8_t line)
{
	lcd_gotoxy(0,line);	/* move cursor to position 8 on line 1 (zero indexed), this is the text after the "status:" text */
	lcd_puts("                    ");/* clear the lcd line with 20x "spaces" */
}

void lcdUpdateSamplingTime(uint16_t elaspsedseconds)
{
	//convert seconds int to string
	char elaspsed_seconds_string[5];
	itoa (elaspsedseconds,elaspsed_seconds_string,10);
	//put string to lcd
	lcd_gotoxy(14,0);	/* move cursor to position 8 on line 1 (zero indexed), this is the text after the "status:" text */
	lcd_puts(elaspsed_seconds_string);/* clear the lcd line with 20x "spaces" */
	lcd_puts("s");/* clear the lcd line with 20x "spaces" */
}

void lcdUpdatePWMValue(uint8_t pwm_value)
{
char sample_pump_pwm_speed_string[4];
itoa(pwm_value,sample_pump_pwm_speed_string,10);
lcd_gotoxy(16,1); // sends lcd cursor position to after the "pumping mPWM " text
lcd_puts(sample_pump_pwm_speed_string);/* put string to display motor PWM value */
}

void lcdUpdatePressureInfo(double filter_pressure,double bag_pressure)
{	
	char filter_pressure_string[17] = "xx.xx";
	char bag_pressure_string[17] = "xx.xx";
	
	sprintf(filter_pressure_string,"pF %2.2f psi", filter_pressure); /**/
	sprintf(bag_pressure_string,"pB %2.2f psi", bag_pressure); /**/
	
	lcdLine2Update(filter_pressure_string);
	lcdLine3Update(bag_pressure_string);
}

void lcdShowIfRunCompleted()
{
	if ((modeFlags & 0b10000000) == 0b10000000)		/* test mSB of modeflags to see if a sample has been collected */
	{
	} 
	else
	{
	lcdLine3Update("SAMPLING COMPLETE");	
	}
}


void printModeflags(void){
	#ifndef LOGGING_ENABLE
		uart_puts(0,"modeFlags = ");                          // to test
		uart_printBinaryByte(0,modeFlags);
		uart_puts(0,"\r\n");
	#endif
	#ifdef LOGGING_ENABLE
		LogModeFlags();
	#endif
}


void LogModeFlags(){
	unsigned char modeFlagscopy = modeFlags;
	static char modeflagsstr[22];
	static char bin[8] = { 1 };
	int i;
	for ( i = 7; i >= 0; i-- ){
		bin[i] = (modeFlagscopy % 2) + '0';
		modeFlagscopy /= 2;
	}
	//printf(bin);
	snprintf(modeflagsstr,22,"Modeflags: %s\r\n",bin);
	LogDebug(modeflagsstr);
}


	


// char* chartobin ( unsigned char c ){
// 	static char bin[CHAR_BIT + 1] = { 0 };
// 	int i;
// 	for ( i = CHAR_BIT - 1; i >= 0; i-- ){
// 		bin[i] = (c % 2) + '0';
// 		c /= 2;
// 	}
// 	return bin;
// }
