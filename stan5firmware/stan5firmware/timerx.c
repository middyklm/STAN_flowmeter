/************************************************************************
* @file   timerx.c
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  16-bit timer with ISR callbacks at 10Hz and 1Hz rates.
* @note   All functions temporarily disable & restore global interrupts.
* @note   Change TIMERX_MODULE definition to choose timer hardware module.
************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <util/atomic.h>
#include "timerx.h"

// alias timer labels to the timer number defined by timerx.h...
#define _TCCRxA CAT( TCCR,Tx)
#define  TCCRxA CAT(_TCCRxA,A)
#define _TCCRxB CAT( TCCR,Tx)
#define  TCCRxB CAT(_TCCRxB,B)
#define _OCRxAH CAT( OCR,Tx)
#define  OCRxAH CAT(_OCRxAH,AH)
#define _OCRxAL CAT( OCR,Tx)
#define  OCRxAL CAT(_OCRxAL,AL)
#define  TIMSKx CAT(TIMSK,Tx)

// these variables are accessed by timerx_isr.h
volatile uint8_t  timerx_sub __attribute__ ((used)); // sub-second counter
volatile uint32_t timerx_sec __attribute__ ((used)); // 1 Hz counter
void (*timerx_callback_sub)(void) __attribute__ ((used)); // pointer to callback function for sub-second interrupt
void (*timerx_callback_sec)(void) __attribute__ ((used)); // pointer to callback function for 1 Hz interrupt

/** 
 * @brief start the timer and enable interrupts
 */
void timerx_start(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		timerx_sub = 0;
		timerx_sec = 0;
		// CTC mode: WGMn3..0 = 0100
		TCCRxA = 0x00;
		TCCRxB = 0x08 | 0x04; // (WGMx2=1) | (CSx = 4 = divide clock by 256)
		OCRxAH = Tx_OCRx >> 8;
		OCRxAL = Tx_OCRx & 0xFF;
		TIMSKx = 1 << OCIE1A; // OCIExA is the same for all timers
	}
}

/** 
 * @brief stop the timer and disable interrupts
 */
void timerx_stop(void){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		TCCRxB = 0x08 | 0x00; // (WGMx2=1) | (CSx = 0 = stopped)
		TIMSKx = 0; // OCIExA is the same for all timers
	}
}

void timerx_set_1hz_callback( void(*fp)() ){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		timerx_callback_sec = fp;
	}
}
void timerx_set_subsec_callback( void(*fp)() ){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		timerx_callback_sub = fp;
	}
}
  

void timerx_time(sec_subsec_t* t){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		t->sec = timerx_sec;
		t->sub = timerx_sub;
	}
}
