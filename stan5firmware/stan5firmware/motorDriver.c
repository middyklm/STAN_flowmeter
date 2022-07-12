/*
 * motorDriver.c
 *
 * Created: 12/09/2020 7:35:13 PM
 *  Author: cra24x
 */ 


#include <stdio.h>
#include <avr/io.h>
#include "uart.h"
#include "logging.h"
#include "motorDriver.h" 


/*		Toshiba TB6593FNG Motor Driver Functions	*/
/*****************************************************/

void MotordriverInit(void){			// this function will need to be adapted depending on how many TB6593FNG motor drivers are used and what pins they are connected to
	
	#ifdef LOGGING_ENABLE
	LogDebug("MotordriverInit() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("MotordriverInit() started\r\n");
	#endif
	//motor driver 1, set data direction registers to make them output pins
	MOTORDRIVER_1_INPUT_1_DDR |= (1<<MOTORDRIVER_1_INPUT_1_PIN);// motor driver 1in1 Data direction register set to output
	MOTORDRIVER_1_INPUT_2_DDR |= (1<<MOTORDRIVER_1_INPUT_2_PIN);
	MOTORDRIVER_1_INPUT_PWM_DDR	|= (1<<MOTORDRIVER_1_INPUT_PWM_PIN);
	MOTORDRIVER_1_INPUT_STBY_DDR	|= (1<<MOTORDRIVER_1_INPUT_STBY_PIN);
	//2nd motor driver is not used in stan firmware v5
	//printf("MotordriverInit() Completed\r\n");
	#ifdef LOGGING_ENABLE
	LogDebug("MotordriverInit() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("MotordriverInit() complete\r\n");
	#endif
	
}

void MotordriverRun(uint8_t motordriver_number, uint8_t direction,int motor_speed_pwm)
{
	#ifdef LOGGING_ENABLE
	LogDebug("MotordriverRun() called\r\n");
	#endif
	#ifdef LOGGING_ENABLE
	printf("MotordriverRun() called\r\n");
	#endif
	switch(motordriver_number)
	{	// this switch statement determines which motor driver the code is acting upon
		case 1  :
		if (direction == 0)// direction 0 = CW on driver
		{
			#ifdef LOGGING_ENABLE
			LogDebug("MotordriverRun() - driver 1 forward (Clockwise) ON\r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"MotordriverRun() - driver 1 forward (Clockwise) ON\r\n");
			#endif
			MOTORDRIVER_1_INPUT_1_PORT		|=	(1<<MOTORDRIVER_1_INPUT_1_PIN);  // set the motor 1 input 1 port to high
			//MOTORDRIVER_1_INPUT_PWM_PORT	|=	(1<<MOTORDRIVER_1_INPUT_PWM_PIN); // set the motor 1 input pwm to high for on off operation, this has been replaced by pwm code below
			MOTORDRIVER_1_INPUT_PWM_OCR      = motor_speed_pwm; // assugn an 8-bit value to the pwm register to set the motor speed via pwm
			MOTORDRIVER_1_INPUT_STBY_PORT	|=	(1<< MOTORDRIVER_1_INPUT_STBY_PIN); // set the motor driver stby to high
			MOTORDRIVER_1_INPUT_2_PORT		&=	~(1<<MOTORDRIVER_1_INPUT_2_PIN);  
		}
		else if (direction == 1)
		{  // CCW on driver = direction 1
			#ifdef LOGGING_ENABLE
			LogDebug("MotordriverRun() - driver 1 reverse (Counter Clockwise) ON\r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"MotordriverRun() - driver 1 reverse (Counter Clockwise) ON\r\n");
			#endif
			MOTORDRIVER_1_INPUT_1_PORT		&=	~(1<<MOTORDRIVER_1_INPUT_1_PIN);  // set the motor 1 input 1 port to low
			//MOTORDRIVER_1_INPUT_PWM_PORT	|=	(1<<MOTORDRIVER_1_INPUT_PWM_PIN); // set the motor 1 input pwm to high,this has been replaced by pwm code below
			MOTORDRIVER_1_INPUT_PWM_OCR      = motor_speed_pwm; // assugn an 8-bit value to the pwm register to set the motor speed via pwm
			MOTORDRIVER_1_INPUT_STBY_PORT	|=	(1<< MOTORDRIVER_1_INPUT_STBY_PIN); // set the motor driver stby to high
			MOTORDRIVER_1_INPUT_2_PORT		|=	(1<<MOTORDRIVER_1_INPUT_2_PIN);  // set the motor driver input 2 to high
			
			}else{
			#ifdef LOGGING_ENABLE
			LogError("invalid direction argument sent to MototdriverInit() for motor driver 1 \r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"invalid direction argument sent to MototdriverInit() for motor driver 1 \r\n");
			#endif
		}
		;   // is this needed, left it here just in case :)
		break;
		
		
		case 2  :
		if (direction == 0)
		{
			#ifdef LOGGING_ENABLE
			LogDebug("MotordriverRun() - driver 2 forward (Clockwise) ON\r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"MotordriverRun() - driver 2 forward (Clockwise) ON\r\n");
			#endif
			// 				MOTORDRIVER_2_PORT	|= (1<<MOTORDRIVER_2_INPUT_1) | (1<<MOTORDRIVER_2_INPUT_PWM) | (1<< MOTORDRIVER_2_INPUT_STBY);
			// 				MOTORDRIVER_2_PORT	&= ~(1<<MOTORDRIVER_2_INPUT_2);
			}else if (direction == 1){
			#ifdef LOGGING_ENABLE
			LogDebug("MotordriverRun() - driver 2 reverse (Counter Clockwise) ON\r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"MotordriverRun() - driver 2 reverse (Counter Clockwise) ON\r\n");
			#endif
			// 				MOTORDRIVER_2_PORT	|= (1<<MOTORDRIVER_2_INPUT_2) | (1<<MOTORDRIVER_2_INPUT_PWM) | (1<< MOTORDRIVER_2_INPUT_STBY);
			// 				MOTORDRIVER_2_PORT	&= ~(1<<MOTORDRIVER_2_INPUT_1);
			}else{
			#ifdef LOGGING_ENABLE
			LogError("invalid direction argument sent to MototdriverInit() for motor driver 2 \r\n");
			#endif
			#ifdef LOGGING_ENABLE
			uart_puts(0,"invalid direction argument sent to MototdriverInit() for motor driver 2 \r\n");
			#endif
		}
		;
		break;
		
		default :
		// error condition: invalid motordriver_number sent to MotordriverRun()
		#ifdef LOGGING_ENABLE
		LogError("invalid motordriver_number sent to MotordriverRun()\r\n");
		#endif
		#ifdef LOGGING_ENABLE
		uart_puts(0,"invalid motordriver_number sent to MotordriverRun()\r\n");
		#endif
		break;
	}
	//printf("MotordriverRun() complete\r\n");
}

void MotordriverStop(uint8_t motordriver_number)
{
	//printf("MotordriverStop() called\r\n");
	switch(motordriver_number) // this switch statement determines which motor driver the code is acting upon
	{
		case 1  :
		#ifdef LOGGING_ENABLE
		LogDebug("MotordriverStop() - driver 1 STOP \r\n");
		#endif
		#ifdef LOGGING_ENABLE
		uart_puts(0,"MotordriverStop() - driver 1 STOP \r\n");
		#endif
		MOTORDRIVER_1_INPUT_1_PORT	&= ~(1<<MOTORDRIVER_1_INPUT_1_PIN); // placing both inputs to the motor driver low will put the motor in stop  mode
		MOTORDRIVER_1_INPUT_2_PORT &= ~(1<<MOTORDRIVER_1_INPUT_2_PIN);
		MOTORDRIVER_1_INPUT_STBY_PORT= (uint8_t)~(1<<MOTORDRIVER_1_INPUT_STBY_PIN);		// placing STBY pin to low will put the motor driver into standby mode, regardl;ess of the state of the other pins
		break;
		case 2  :
		LogDebug("MotordriverStop() - driver 2 STOP *** this code is incomplete** \r\n");
		//uart_puts(0,"MotordriverStop() - driver 2 STOP *** this code is incomplete** \r\n");
		//MOTORDRIVER_2_PORT	&= ~((1<<MOTORDRIVER_2_INPUT_1) | (1<<MOTORDRIVER_2_INPUT_2)); // placing both inputs to the motor driver hlow will put the motor in stop  mode
		//MOTORDRIVER_2_PORT	&= ~(1<<MOTORDRIVER_2_INPUT_STBY);		// placing STBY pin to low will put the motor driver into standby mode, regardl;ess of the state of the other pins
		break;
		default :
		// error condition: invalid motordriver_number sent to MotordriverStop()
		#ifdef LOGGING_ENABLE
		LogError("invalid motordriver_number sent to MotordriverStop()\r\n");
		#endif
		#ifdef LOGGING_ENABLE
		uart_puts(0,"invalid motordriver_number sent to MotordriverStop()\r\n");
		#endif
		break;
	}
}


