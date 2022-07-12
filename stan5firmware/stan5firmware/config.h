/************************************************************************
* @file   config.h
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  define global flags to control compilation options.
************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include "fs2012.h"
#include "pulseflow.h"

// comment out this line to use older PCB without LCD...
//#define LCD_ENABLE

// comment out this line if not using fram.h code
//#define FRAM_ENABLE

// comment out this line if not using logging.h code
//#define LOGGING_ENABLE

// uncomment this line to demo the FS2012 flow rate sensor..
//#define DEMO_FLOW_RATE_SENSOR
//#define USE_PULSE_FLOW_SENSOR

// tune these to realistic values
#define PULSE_PER_ML_NUMERATOR 731L
#define PULSE_PER_ML_DENOMINATOR 263L

// define the 16-bit hardware timer to use for timerx.h module...
#define TIMERX_MODULE 4

// define cutoff parameters
#define CUTOFF_VOLUME_MAX_ML (2000UL)
#define CUTOFF_STALLED_SECONDS (30)

// cut off if at least this volume received and flow drops below a threshold
#define CUTOFF_LOWRATE_MIN_VOL_ML (CUTOFF_VOLUME_MAX_ML/5)
#define CUTOFF_LOWRATE_MIN_RATE_PEAK_ML_PER_MINUTE (FLOW_CALIBRATED_MIN)
#define CUTOFF_LOWRATE_MIN_RATE_INST_ML_PER_MINUTE (FLOW_CALIBRATED_MIN/2)

// define an array of callback functions to run in the 1Hz callback (each one called in this order, if enabled). 
// TIMERX_1HZ_CALLBACKS is an array of selectable_callback_t (first value in struct is a function pointer, second is pointer to an enabled flag)
// TIMERX_1HZ_CALLBACK_COUNT is the number of elements in TIMERX_1HZ_CALLBACKS. 
#define TIMERX_1HZ_CALLBACKS { { &paddle_callback_1Hz, &pulseFlowEnabled} } 
#define TIMERX_1HZ_CALLBACK_COUNT (1)

#define PULSE_ISR_VECTOR (INT6_vect)

#endif /* CONFIG_H_ */