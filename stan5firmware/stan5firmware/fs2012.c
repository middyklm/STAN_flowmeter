/************************************************************************
* @file   fs2012.c
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  Driver for FS2012 flow sensor.
* @note   Requires main.c to #include "timerx_isr.h"
************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>
#include <util/delay.h>
#include "i2c.h"
#include "timerx.h"
#include "fs2012.h"
#include "sync.h"
#include "config.h"

#define FS2012_ADDRESS (0x07)
#define I2C_READ (0x01)

// Parameters for exponentially-weighted moving average.
// For values EXP_PREV_FACTOR = 3, EXP_DENOMINATOR = 4, EXP_OFFSET = 1, the exponentially weighted moving average will settle fully by by 1.8 seconds after step-change.
// Because of integer maths, the precision of settled average is +1
#define EXP_PREV_FACTOR (3)
#define EXP_DENOMINATOR (4)
#define EXP_OFFSET (1)

////////////////////////////////////////////////////////////////
// internal state variables
volatile uint16_t reads_total;
volatile uint16_t reads_good;
volatile uint16_t reads_inrange;
volatile uint16_t reads_bad;
volatile uint16_t reads_bad_consecutive;
volatile uint16_t flow_units;
volatile uint16_t flow_exp_avg;
volatile uint16_t flow_exp_peak;
volatile uint16_t subsec_count; // number of subsecs to multiply flow_units by when adding to flow_accum;
volatile unsigned long flow_accum;
bool is_running;

////////////////////////////////////////////////////////////////
// internal function declarations...
void fs2012_subsec(void); // subsecond callback function
uint16_t fs2012_sample(void); // sample sensor

////////////////////////////////////////////////////////////////
// external API function implementations... 

/**
 * @brief reset values and start timers (enables ISR).
 */
void flow_sensor_start(void){
	timerx_stop();
	timerx_set_subsec_callback(&fs2012_subsec);
	//timerx_set_1hz_callback(&fs2012_1Hz); // actually don't need this
	
	reads_total = 0;
	reads_good = 0;
	reads_bad = 0;
	reads_bad_consecutive = 0;
	
	flow_units = 0;
	flow_exp_avg = 0;
	flow_exp_peak = 0;
	flow_accum = 0;
	//flow_snapshot();
	subsec_count = 0;
	timerx_start();
	is_running = true;
}

/**
 * @brief stop operating FS2012 (disables timer ISRs).
 */
void flow_sensor_stop(void){
	timerx_stop();
	is_running = false;
	timerx_set_subsec_callback(NULL);
}

/**
 * @brief returns true if flow sensor is running.
 */
inline bool flow_sensor_running(void){
	return is_running;
}

/**
 * @brief get instantaneous flow rate, in FS2012 native units (read-copy is atomic)
 * @note temporarily disables (then restores) interrupt for atomic copy 
 */
uint16_t flow_rate_now(void){
	return atomic_read_u16((uint16_t*)&flow_units);
}


/**
 * @brief returns true if the target sample volume has been achieved (specifically, if flow_net_mL() > CUTOFF_VOLUME_MAX_ML)
 */
bool flow_sample_cutoff_volume(void){
	return (flow_net_mL() > CUTOFF_VOLUME_MAX_ML);
}

/**
 * @brief returns true if a partial sample has been taken (flow_net_mL() > CUTOFF_LOWRATE_MIN_VOL_ML), flow reached a peak rate (flow_exp_peak >= CUTOFF_LOWRATE_MIN_RATE_PEAK_ML_PER_MINUTE), and the flow rate has fallen below threshold (flow_exp_avg  <  CUTOFF_LOWRATE_MIN_RATE_INST_ML_PER_MINUTE).
 */
bool flow_sample_cutoff_lowrate(void){
	return (
		(flow_net_mL() > CUTOFF_LOWRATE_MIN_VOL_ML) &&											// flow total volume is at least this volume
		(atomic_read_u32((uint32_t*)&flow_exp_peak) >= CUTOFF_LOWRATE_MIN_RATE_PEAK_ML_PER_MINUTE) &&		// flow rate peak at some point reached at least this rate
		(atomic_read_u32((uint32_t*)&flow_exp_avg)  <  CUTOFF_LOWRATE_MIN_RATE_INST_ML_PER_MINUTE)			// flow rate now has fallen to this cutoff rate
	);
}


/**
 * @brief get integrated flow since flow_sensor_start(), in FS2012 native units * subseconds (i.e. divide by (60*10*10=6000) to get value in mL).
 * 1 NU = 1 native unit
 * rate: 10 subsec/sec or 0.1 sec/subsec
 * 5000 NU = 500 mL/min = 8.3333 mL/sec = 0.8333 mL/subsec
 * 5000/0.8333 [NU.subsec] = 1 mL
 * 6000 [NU.subsec] = 1 mL
 * 6 [NU.subsec] = 0.001 mL
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long  flow_net_unitsubsec(void){
	return atomic_read_u32((uint32_t*)&flow_accum);
}

/**
 * @brief get integrated flow since flow_sensor_start(), in microlitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long flow_net_uL(void){
	return atomic_read_u32((uint32_t*)&flow_accum)/6;
}

/**
 * @brief get integrated flow since flow_sensor_start(), in millilitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint16_t flow_net_mL(void){
	return (uint16_t)(atomic_read_u32((uint32_t*)&flow_accum)/6000L);
}

/**
 * @brief get integrated flow since flow_sensor_start(), in millilitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint16_t flow_peak_mL_per_minute(void){
	return (uint16_t)(atomic_read_u32((uint32_t*)&flow_exp_peak));
}

/**
 * @brief get integrated flow since flow_sensor_start(), in millilitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
//unsigned long flow_net_mL(void){
//	flow_snapshot();
//	return (flow_accum_buffered/6000); 
//}

////////////////////////////////////////////////////////////////
// internal functions bodies...

void fs2012_subsec(void){
	flow_units = fs2012_sample();
	reads_total++;
	subsec_count++;
	if (flow_units == FLOW_BAD_SAMPLE){
		reads_bad++;
		reads_bad_consecutive++;
		return;
	}
	else{
		reads_good++;
		if(reads_good>1){ // don't integrate the first valid reading
			flow_accum += flow_units * subsec_count; // multiply this valid reading by number of subseconds since previous valid reading.
			
			flow_exp_avg = (flow_exp_avg * EXP_PREV_FACTOR + flow_units + EXP_OFFSET) / EXP_DENOMINATOR ;
			if (flow_exp_avg > flow_exp_peak){
				flow_exp_peak = flow_exp_avg;
			}
			if(flow_units >= FLOW_CALIBRATED_MIN){
				reads_inrange++;
			}
		}
		subsec_count = 0; // reset
		reads_bad_consecutive = 0;
	}
}


uint16_t fs2012_sample(void){
	
	uint16_t data;
	uint8_t tmp;
	initI2C();
	
	// use a timeout in case chip is non-responsive or 5V supply fails
	i2cSetTimeout(I2C_MODE_TIMEOUT, I2C_TIMEOUT_1MS);
	
	// write start condition
	i2cStart(); 
	
	// send FS2012's I2C address, with read bit set
	i2cSend( (FS2012_ADDRESS<<1) | I2C_READ );
	
	// read back one byte, acknowledge
	tmp = i2cReadAck();
	if (i2cGotTimeout())
		return FLOW_BAD_SAMPLE;
	data = tmp;
	data = data<<8;
	
	// read back another byte, no acknowledge required
	tmp = i2cReadNoAck();
	if (i2cGotTimeout())
		return FLOW_BAD_SAMPLE;
	data |= tmp;
	
	// stop condition
	i2cStop();
	
	// limit to maximum flow condition
	if(data>FLOW_OVER_RANGE)
		return FLOW_OVER_RANGE;
		
	return data;
}

