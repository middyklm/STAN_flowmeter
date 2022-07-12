/*
 * sync.c
 *  Author: hug159
 */ 

#include <stdint.h>
#include <util/atomic.h>
#include "sync.h"

uint16_t atomic_read_u16(uint16_t* p){
	uint16_t ret;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ret = *p;
	}
	return ret;
}
uint32_t atomic_read_u32(uint32_t* p){
	uint32_t ret;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ret = *p;
	}
	return ret;
}
