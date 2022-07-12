/************************************************************************
* @file   timerx_int.h
* @date   4/04/2019
* @author daniel.hugo@csiro.au
* @brief  Include in main.c to provide ISR body for timerx module.
************************************************************************/

#ifndef TIMERX_INT_H_
#define TIMERX_INT_H_

#include <stddef.h>
#include <stdint.h>
#include "timerx.h"
#include "config.h"

// paste together alias TIMERx_COMPA_vect = TIMER + TX + _COMPA_vect
#define _TIMERx_COMPA_vect CAT(TIMER,Tx)
#define  TIMERx_COMPA_vect CAT(_TIMERx_COMPA_vect,_COMPA_vect)

// extern variables are implemented in timerx.c...
extern uint8_t  timerx_sub; // sub-second counter
extern uint32_t timerx_sec; // 1 Hz counter
extern void (*timerx_callback_sub)(void); // pointer to callback function for sub-second interrupt
extern void (*timerx_callback_sec)(void); // pointer to callback function for 1 Hz interrupt

volatile selectable_callback_t timerx_1hz_callbacks[] = TIMERX_1HZ_CALLBACKS ;

// run every time the configured timer triggers  
// low rate: call timerx_callback_sec function and/or any callbacks in the timerx_1hz_callbacks array once per second.
// high rate: call the timerx_callback_sub function TIMERx_HZ times per second. 
ISR (TIMERx_COMPA_vect){
	timerx_sub = (timerx_sub+1) % TIMERx_HZ ;
	if (timerx_sub == 0){
		timerx_sec++;
		if (timerx_callback_sec!=NULL) (*timerx_callback_sec)(); // call 1 Hz callback, if set
		for(int i = 0; i< TIMERX_1HZ_CALLBACK_COUNT; i++){
			selectable_callback_t cb = timerx_1hz_callbacks[i];
			if(cb.enabled)
			cb.fp();
		}
	}
	else{
		if (timerx_callback_sub!=NULL) (*timerx_callback_sub)(); // call sub-second callback, if set
	}
}


#endif /* TIMERX_INT_H_ */