/************************************************************************
* @file   fs2012.h
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  Driver for FS2012 flow sensor. 
* @note   Requires main.c to #include "timerx_isr.h"
************************************************************************/

/* Conversions:
native units...
5000 unit = 500mL / 60sec = 8.333 mL/sec
 600 unit =  60mL / 60sec = 1.000 mL/sec
  10 unit =   1mL / 60sec = 0.0167mL/sec

native units x seconds...
5000 unit.sec = 500/60 mL = 8.333 mL
 600 unit.sec =  60/60 mL = 1.000 mL
  10 unit.sec =   1/60 mL = 0.0167mL

multiply LHS by (10 subseconds per sec)...
50,000 unit.subsec = 8.333 mL
 6,000 unit.subsec = 1.000 mL
   100 unit.subsec = 0.0167mL

*/

#ifndef FS2012_H_
#define FS2012_H_

#include <stdint.h>
#include <stdbool.h>

#define FLOW_BAD_SAMPLE (0xffff)

#define FLOW_OVER_RANGE (5000) // was 5001. Equal to 500 mL/minute
#define FLOW_CALIBRATED_MIN (500) // Equal to 50 mL/minute

/**
 * @brief reset integration start time and accumulted value, start timers.
 */
void flow_sensor_start(void);

/**
 * @brief stop operating FS2012 (disables timer ISRs).
 */
void flow_sensor_stop(void);

/**
 * @brief returns true if flow sensor is running
 */
bool flow_sensor_running(void);

/**
 * @brief returns true if the target sample volume has been achieved (specifically, if flow_net_mL() > CUTOFF_VOLUME_MAX_ML)
 */
bool flow_sample_cutoff_volume(void);

/**
 * @brief returns true if a partial sample has been taken (flow_net_mL() > CUTOFF_LOWRATE_MIN_VOL_ML), flow reached a peak rate (flow_exp_peak >= CUTOFF_LOWRATE_MIN_RATE_PEAK_ML_PER_MINUTE), and the flow rate has fallen below threshold (flow_exp_avg  <  CUTOFF_LOWRATE_MIN_RATE_INST_ML_PER_MINUTE).
 */
bool flow_sample_cutoff_lowrate(void);

/**
 * @brief get instantaneous flow rate, in FS2012 native units
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint16_t flow_rate_now(void);

/**
 * @brief get integrated flow since last flow_sensor_start(), in FS2012 native units * subseconds
 * @note 6000 [unit x subsec] = 1 mL.
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long flow_net_unitsubsec(void);

/**
 *  brief get integrated flow since last flow_sensor_start(), in microlitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long flow_net_uL(void);

/**
 * @brief get integrated flow since flow_sensor_start(), in millilitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint16_t flow_net_mL(void);

void fs2012_subsec(void);
uint16_t fs2012_sample(void);
#endif /* FS2012_H_ */
