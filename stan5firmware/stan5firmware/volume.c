/*
 * volume.c
 *
 * Created: 29/03/2022 11:22:25 AM
 *  Author: Middy Khong
 */ 

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

#include "r9valve.h"
#include "motorDriver.h"
#include "uart.h"
#include "volume.h"
#include "fs2012.h"
#include "settings.h"

#define PUMP1				1
#define CLOCKWISE			0
#define COUNTER_CLOCKWISE	1

settings_Union settings;

void stopTimer()
{
	TCCR1B = 0;
	TIMSK1 = 0;
}


uint16_t flowrate_mean(uint16_t flowrates[], uint8_t n){
	uint8_t i = 0;
	uint32_t sum = 0;
	uint16_t mean = 0;

	for (i=0; i<n; i++){
		sum = sum + flowrates[i];
	}
	
	mean = sum/n;
	
	return mean;
}


float flowrate_to_volume(uint16_t q, uint8_t i, uint8_t n){
	return q*i/(float)60/(float)n/(float)10;
}

void set_sample_volume(uint8_t loLimit, uint16_t hiLimit){
	uint8_t maxChar = 10;
	char strVol[maxChar];
	uint16_t intVol = 0;
	
	do{
		printf("Enter sample volume (ml): ");
		UART0_getStr(strVol, maxChar);
		intVol = atoi(strVol);
		printf("\r\nEntry = %u ml\r\n", intVol);
	} while (intVol<loLimit || intVol>hiLimit);
	
	settings.volumeSample = intVol;
	settings_Save(settings);
}

struct vol volume_instants(struct vol v){
	
	if (TCNT1 > v.i1*v.sample_interval && v.i1<v.n_meas)
	{
		
		v.flowrates[v.i1] = fs2012_sample();
		v.volume_instant += flowrate_to_volume(v.flowrates[v.i1], v.i_meas, v.n_meas);
		if (v.i1 > v.n_meas-v.n_ovlp)
		{
			v.flowrates_ol[v.i3] = v.flowrates[v.i1];
			v.i3++;
		}
	
		v.i1++;
	}
	
	return v;	
}

struct vol volume_means(struct vol v){
	
	if (v.i2 >= v.i_meas - v.i_ovlp)
	{
		v.flr_mean = flowrate_mean(v.flowrates, v.n_meas);
		v.volume_mean += flowrate_to_volume(v.flr_mean, v.i_meas, 1);

		for (v.i1 = 0; v.i1<v.n_ovlp; v.i1++){
			v.flowrates[v.i1] = v.flowrates_ol[v.i1];
		}
				
		v.i2 = 0;
		v.i3 = 0;
	}
		
	return v;	
}

struct vol volume_init(struct vol v){
	v.i_meas = INTERVAL_MEAS;
	v.i_ovlp = INTERVAL_OVLP;
	v.n_meas = N_SAMPLES_MEAS;	
	v.n_ovlp = v.i_ovlp * v.n_meas / v.i_meas;
	v.flowrates[v.n_meas] = 0;	
	v.flowrates_ol[v.n_ovlp] = 0;
	v.sample_interval = 31250 * v.i_meas / v.n_meas;
	v.flr_mean = 0;	
	v.volume_instant = 0;	
	v.volume_mean = 0;	
	v.i1 = 0;	
	v.i2 = 0;
	v.i3 = 0;
	v.i5 = 0;	
	return v;
}

