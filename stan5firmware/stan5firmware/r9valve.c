/*
 * r9valve.c
 *
 * Created: 12/09/2020 8:54:28 PM
 *  Author: cra24x
 */ 


#include <avr/io.h>
#include "stdio.h"
#include <util/delay.h>

// Valve Defines
#define INLET_VALVE_PWM_PIN	OCR5C

void inlet_valve_denergise(void){
	#ifdef LOGGING_ENABLE
		LogDebug("inlet_valve_denergise() called\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("inlet_valve_denergise() called\r\n");
	#endif
	/*set pwm pin PH6/OCR2B to 0 to deenergise the solenoid*/
	INLET_VALVE_PWM_PIN = 0; // set the 8-bit pwm value for the valve driver 0-255
}

void inlet_valve_energise(void){
	#ifdef LOGGING_ENABLE
		LogDebug("inlet_valve_energise() called\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		printf("inlet_valve_energise() called\r\n");
	#endif
	//printf("inlet_valve_energise() called\r\n");
	/*activate hit and hold for parker r9 valve set pwm pin PH6/OCR2B to full (255/255, 12V) for 30ms then to half (128/255, 6V) to hold*/
	INLET_VALVE_PWM_PIN = 255; // set the 8-bit pwm value for the valve driver 0-255
	_delay_ms(250);  //************* set at 250ms, lengthened for testingn
	INLET_VALVE_PWM_PIN = 140; // set the 8-bit pwm value for the valve driver 0-255
	//_delay_ms(2500);  //************* only here for testing, not require 
}

