/************************************************************************
* @file   timerx.h
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  16-bit timer with ISR callbacks at 10Hz and 1Hz rates.
* @note   All functions temporarily disable & restore global interrupts.
* @note   Change TIMERX_MODULE definition to choose timer hardware module.
************************************************************************/

#ifndef TIMERX_H_
#define TIMERX_H_

// choose timer module by changing this define...
#ifdef TIMERX_MODULE
#define Tx TIMERX_MODULE
#else
//#define Tx 1
//#define Tx 3
#define Tx 4
//#define Tx 5
#endif

// preprocessor double-paste trick...
#define _CAT(x,y) x ## y
#define  CAT(x,y) _CAT(x,y)

// timerx module interrupt rate Hz...
#define TIMERx_HZ 10

// 8MHZ / 256 prescaler / 10 Hz = 3125  
#define CLK_HZ   8000000
#define Tx_PRESC 256
#define Tx_OCRx  ( CLK_HZ / Tx_PRESC / TIMERx_HZ )

// This is the basic time type, in seconds and sub-seconds (where each sub-second is 1/TIMERx_HZ of one second)
typedef struct __attribute__ ((packed)) sec_subsec_ {
	uint32_t sec;
	uint8_t sub;
	} sec_subsec_t;

// A selectable callback is a function pointer and an enable flag
typedef struct __attribute__ ((packed)) selectable_callback {
	void(*fp)() ;
	volatile uint8_t *  enabled;
} selectable_callback_t;

/** 
 * @brief start the timer and enable interrupts
 */
void timerx_start(void);

/** 
 * @brief stop the timer and disable interrupts
 */
void timerx_stop(void);

void timerx_set_1hz_callback( void(*fp)() );
void timerx_set_subsec_callback( void(*fp)() );

/** 
 * @brief set the pointed-to address to the current elapsed time according to timerx.
 */
void timerx_time(sec_subsec_t* t);



#endif /* TIMERX_H_ */