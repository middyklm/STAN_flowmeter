/*
 * buoyancy.c
 *
 * Created: 11/09/2020 2:48:49 PM
 *  Author: cra24x
 */ 


#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "motorDriver.h"
#include "pressureSensor.h"

#define BUOYANCY_PUMP	1
#define PUMP_OUT		0
#define PUMP_IN			1

#define PRESSURE_SENSOR_INTERNAL	  1 
#define PRESSURE_SENSOR_EXTERNAL      2
#define PRESSURE_SENSOR3				3



void BUOYANCYgoToSurface(void){	//pump fluid to external bag to a set differential pressure until the surface is reached
	 //double internal_pressure,external_pressure, target_pressure;
//Check external pressure and calculate depth
	 //external_pressure = ReadPressure(PRESSURE_SENSOR_EXTERNAL);
//check internal pressure and set desired internal pressure to target
	 //internal_pressure = ReadPressure(PRESSURE_SENSOR_INTERNAL);
//run pump to ascend
	 printf("Going to surface..\r\n");
	 MotordriverRun(BUOYANCY_PUMP,PUMP_OUT,255);// pump on CW
	 _delay_ms(10000);
	 MotordriverStop(BUOYANCY_PUMP);	/* turn off pump (pump2) */
	 printf("Stop for 10 s\r\n");
	 _delay_ms(10000);	 
}


void BUOYANCYgoToDepth(int target_depth){
	printf("Going down..\r\n");
	MotordriverRun(BUOYANCY_PUMP,PUMP_IN,255);
	_delay_ms(10000);
	MotordriverStop(BUOYANCY_PUMP);	
	printf("Stop for 10 s\r\n");
	_delay_ms(10000);
}