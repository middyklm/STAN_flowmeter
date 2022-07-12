/*
 * pressureSensor.h
 *
 * Created: 12/09/2020 8:38:53 PM
 *  Author: cra24x
 */ 


#ifndef PRESSURESENSOR_H_
#define PRESSURESENSOR_H_

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>


// Honeywell Pressure sensors
void PressureSensorInit(void);
double ReadPressure(uint8_t sensor_id);




#endif /* PRESSURESENSOR_H_ */