/*
 * motorDriver.h
 *
 * Created: 12/09/2020 7:34:52 PM
 *  Author: cra24x
 */ 


#ifndef MOTORDRIVER_H_
#define MOTORDRIVER_H_




// MOTOR DRIVER TB6593FNG
#define ON					1
#define OFF					0
#define CLOCKWISE			0		// clockwise
#define COUNTER_CLOCKWISE	1		// counter clockwise
#define SAMPLE_PUMP			1
#define PRESERVATIVE_PUMP	2

//  pump 1 sample
#define MOTORDRIVER_1_INPUT_1_PIN		PG3
#define MOTORDRIVER_1_INPUT_1_DDR		DDRG
#define MOTORDRIVER_1_INPUT_1_PORT		PORTG

#define MOTORDRIVER_1_INPUT_2_PIN		PH7
#define MOTORDRIVER_1_INPUT_2_DDR		DDRH
#define MOTORDRIVER_1_INPUT_2_PORT		PORTH

#define MOTORDRIVER_1_INPUT_PWM_PIN		PL3
#define MOTORDRIVER_1_INPUT_PWM_DDR		DDRL
#define MOTORDRIVER_1_INPUT_PWM_PORT	PORTL
#define MOTORDRIVER_1_INPUT_PWM_OCR		OCR5A

#define MOTORDRIVER_1_INPUT_STBY_PIN	PB7
#define MOTORDRIVER_1_INPUT_STBY_DDR	DDRB
#define MOTORDRIVER_1_INPUT_STBY_PORT	PORTB





// MOTOR DRIVER TB6593FNG Functions
void MotordriverInit(void);
void MotordriverRun(uint8_t motordriver_number, uint8_t direction, int motor_speed_pwm);
void MotordriverStop(uint8_t motordriver_number);


#endif /* MOTORDRIVER_H_ */