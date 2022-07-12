
#ifndef PADDLE_H_
#define PADDLE_H_
#include <stdbool.h>

uint16_t pulse_to_mL(uint16_t count);
uint16_t mL_to_pulse(uint16_t mL);

/**
 * @brief reset integration start time and accumulted value, start timers.
 */
void paddle_sensor_start(void);

/**
 * @brief stop operating paddle sensor (disables timer ISRs).
 */
void paddle_sensor_stop(void);

/**
 * @brief returns true if paddle sensor is running
 */
bool paddle_sensor_running(void);

/**
 * @brief returns true if the target sample volume has been achieved
 */
bool paddle_sample_cutoff_volume(void);

/**
 * @brief returns true if a partial sample has been taken and the flow rate has fallen below threshold
 */
bool paddle_sample_cutoff_lowrate(void);

/**
 * @brief get (near) instantaneous flow rate, in paddle native units
 */
uint16_t paddle_rate_now(void);

/**
 * @brief get integrated flow since last paddle_sensor_start(), in paddle 
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
uint32_t paddle_net_impulses(void);

/**
 *  brief get integrated flow since last paddle_sensor_start(), in microlitres
 * @note temporarily disables (then restores) interrupt for atomic copy
 */
unsigned long paddle_net_uL(void);


uint16_t paddle_net_mL(void);

void paddle_callback_1Hz(void);
extern volatile uint8_t pulseFlowEnabled; 

/**
 * Struct to hold counter stats
 */
typedef struct counter_ {
	volatile uint16_t pcint_counter;
	volatile uint32_t pcint_total;
	volatile uint8_t pps;
	volatile uint8_t pps_avg;
	volatile uint16_t pps_last;
} counter_t;

#endif /* PADDLE_H_ */