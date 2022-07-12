/************************************************************************
* @file   pulseflow.c
* @date   17/03/2021
* @author daniel.hugo@csiro.au
* @brief  Driver for paddle wheel flow sensor.
* @note   Requires main.c to #include "timerx_isr.h"
************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pulseflow.h"
#include "config.h"
#include "wakeupINT2.h"
/*
volatile uint16_t pcint11_counter;	// count changes on PCINT11 pin
volatile uint16_t pcint12_counter;	// count changes on PCINT12 pin

volatile uint32_t pcint11_total;	// count changes on PCINT11 pin
volatile uint32_t pcint12_total;	// count changes on PCINT12 pin

volatile uint8_t pps_11;
volatile uint8_t pps_12;

volatile uint8_t pps_11_avg; // exponential average (90% in 4 sec)
volatile uint8_t pps_12_avg; // exponential average (90% in 4 sec)


volatile uint16_t pps_12_last;*/

volatile uint32_t pulse_total_uL;
volatile uint8_t pulseFlowEnabled = 0;
volatile uint32_t uL_total; 
uint16_t pulseCutoff;
uint8_t secondsLow; 
uint8_t lowrateCutoff; 

counter_t counter_11;
counter_t counter_12;
counter_t counter_13;

/**
 * @return the pulse count equivalent to the given volume in mL.
 */
uint16_t mL_to_pulse(uint16_t mL){
	uint32_t c = mL;
	c *= PULSE_PER_ML_NUMERATOR;
	c /= PULSE_PER_ML_DENOMINATOR;
	if(c>UINT16_MAX){
		c=UINT16_MAX;
	}
	return c;
}

/**
 * @return the volume in mL equivalent to the pulse count given.
 */
uint16_t pulse_to_mL(uint16_t count){
	uint32_t c = count;
	c *= PULSE_PER_ML_DENOMINATOR;
	c /= PULSE_PER_ML_NUMERATOR;
	if(c>UINT16_MAX){
		c=UINT16_MAX;
	}
	return c;
}

void resetCounter(counter_t* counter){
	counter->pcint_counter = 0;
	counter->pcint_total = 0;
	counter->pps = 0;
	counter->pps_avg = 0;
	counter->pps_last = 0;
}

/**
 * @brief reset integration start time and accumulted value, start timers.
 */
void paddle_sensor_start(void){
	cli();
	resetCounter(&counter_11);
	resetCounter(&counter_12);
	resetCounter(&counter_13);

	pulseCutoff = mL_to_pulse(CUTOFF_VOLUME_MAX_ML);
	secondsLow = 0;
	pulse_total_uL = 0;
	
	// TODO activate PL7 (+5v shdn) here
	
	DDRJ &= ~(1<<PJ2); // input
	DDRJ &= ~(1<<PJ3); // input
	DDRJ &= ~(1<<PJ4); // input
	PORTJ|= (1<<PJ2) | (1<<PJ3)| (1<<PJ4); // enable weak pullups
	pin_change_1_init();
	pulseFlowEnabled = 1;
	sei();
}

/**
 * @brief stop operating paddle sensor (disables timer ISRs).
 */
void paddle_sensor_stop(void){
	// TODO de-activate PL7 (+5v shdn) here
	pulseFlowEnabled = 0;
}

/**
 * @brief returns true if paddle sensor is running
 */
bool paddle_sensor_running(void){
	return pulseFlowEnabled>0;
}

/**
 * @brief returns true if the target sample volume has been achieved
 */
bool paddle_sample_cutoff_volume(void){
	return counter_11.pcint_counter >= pulseCutoff;
}

/**
 * @brief returns true if a partial sample has been taken and the flow rate has fallen below threshold (TODO, not implemented)
 */
bool paddle_sample_cutoff_lowrate(void){
	return 0;
}

/**
 * @brief get (near) instantaneous flow rate, in paddle native units
 */
uint16_t paddle_rate_now(void){
	uint16_t rate;
	cli();
	//rate = pps_11_last;
	rate = counter_11.pps_last;
	sei();
	return rate;
}

/**
 * @brief get integrated flow since last paddle_sensor_start(), in paddle 
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint32_t paddle_net_impulses(void){
	uint32_t net;
	cli();
	//net = pcint11_total;
	net = counter_11.pcint_total;
	sei();
	return net;
}

/**
 *  brief get integrated flow since last paddle_sensor_start(), in microlitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long paddle_net_uL(void){
	uint32_t net;
	cli();
	net = uL_total;
	sei();
	return net;
	
}

/**
 * @brief get integrated flow since paddle_sensor_start(), in millilitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint16_t paddle_net_mL(void){
	return pulse_to_mL(counter_11.pcint_total) ;
}

inline void update_avg(volatile uint8_t* avg, uint16_t* new){
	uint16_t val = *new;
	if(val>32767)
		val = 32767;
	val += *avg;
	val /= 2;
	if(val>255)
		val = 255;
	*avg = val;
}

void paddle_callback_1Hz(void){
	cli();
	counter_11.pps_last = counter_11.pcint_counter;
	counter_12.pps_last = counter_12.pcint_counter;
	counter_13.pps_last = counter_13.pcint_counter;
	
	counter_11.pcint_counter = 0;
	counter_12.pcint_counter = 0;
	counter_13.pcint_counter = 0;
	sei();
	
	update_avg(&(counter_11.pps_avg), (uint16_t*) &(counter_11.pps_last));
	update_avg(&(counter_12.pps_avg), (uint16_t*) &(counter_12.pps_last));
	update_avg(&(counter_13.pps_avg), (uint16_t*) &(counter_13.pps_last));
	
	counter_11.pcint_total += counter_11.pps_last;
	counter_12.pcint_total += counter_12.pps_last;
	counter_13.pcint_total += counter_13.pps_last;
	//uart_puts(0,"hi Middy\r\n");
	uint16_t uL_last_second;
	if(counter_11.pps_last >0){
		uL_last_second = 134 * counter_11.pps_last + 638;
	} else {
		uL_last_second = 0;
	}
	uL_total += uL_last_second;
	
	if(counter_11.pps_last == 0){
		secondsLow ++; 
	} else if (secondsLow>0) {
		secondsLow --; 
	}
}
