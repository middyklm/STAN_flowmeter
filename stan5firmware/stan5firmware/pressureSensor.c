/*
 * pressureSensor.c
 *
 * Created: 12/09/2020 8:38:34 PM
 *  Author: cra24x
 */ 


#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "spi.h"


//  Honeywell Pressure sensor macros
#define PRESSURE_SENSOR1           1
#define PRESSURE_SENSOR2           2
#define PRESSURE_SENSOR3           3
#define PRESSURE_SENSOR_SUPPLY_PIN	0			/* defines the pin that supplies power to the pressure sensor*/
#define PRESSURE_SENSOR_SUPPLY_PORT PORTB
#define PRESSURE_SENSOR_SUPPLY_DDR  DDRB
#define PRESSURE_SENSOR1_PIN       4
#define PRESSURE_SENSOR1_PORT      PORTH
#define PRESSURE_SENSOR1_DDR       DDRH
#define PRESSURE_SENSOR2_PIN       5
#define PRESSURE_SENSOR2_PORT      PORTH
#define PRESSURE_SENSOR2_DDR       DDRH
#define PRESSURE_SENSOR3_PIN       6
#define PRESSURE_SENSOR3_PORT      PORTH
#define PRESSURE_SENSOR3_DDR       DDRH
#define PRESSURE_SENSOR_outputMAX		16383
#define PRESSURE_SENSOR_outputMIN		0
#define PRESSURE_SENSOR_pressureMAX		150
#define PRESSURE_SENSOR_pressureMIN		0


/*		Honeywell Pressure sensor Functions	*/
/*****************************************************/

void PressureSensorInit(void){
	
	#ifdef LOGGING_ENABLE
	LogDebug("PressureSensorInit() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("PressureSensorInit() started\r\n");
	#endif
	// initialise pressure sensor power supply pins
	PRESSURE_SENSOR_SUPPLY_DDR |= (1<< PRESSURE_SENSOR_SUPPLY_PIN);		/* Power to the pressure sensors is via a GPIO - set these to output mode*/
	//PRESSURE_SENSOR_SUPPLY_PORT |= (1<< PRESSURE_SENSOR_SUPPLY_PIN);	/* Power to the pressure sensors is turned on*/
	// initialise pressure sensor 1
	PRESSURE_SENSOR1_DDR |= (1<< PRESSURE_SENSOR1_PIN); /* Slave select pins for pressure sensors set to output */
	PRESSURE_SENSOR1_PORT |= (1<< PRESSURE_SENSOR1_PIN); /* sturn slave select pin on to dessert this device until required*/
	// initialise pressure sensor 2
	PRESSURE_SENSOR2_DDR |= (1<< PRESSURE_SENSOR2_PIN); /* Slave select pins for pressure sensors set to output */
	PRESSURE_SENSOR2_PORT |= (1<< PRESSURE_SENSOR2_PIN); /* sturn slave select pin on to dessert this device until required*/
	// // initialise pressure sensor 3
	PRESSURE_SENSOR3_DDR |= (1<< PRESSURE_SENSOR3_PIN); /* Slave select pins for pressure sensors set to output */
	PRESSURE_SENSOR3_PORT |= (1<< PRESSURE_SENSOR3_PIN); /* sturn slave select pin on to dessert this device until required*/
	#ifdef LOGGING_ENABLE
	LogDebug("PressureSensorInit() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("PressureSensorInit() complete\r\n");
	#endif
}

double ReadPressure(uint8_t sensor_id){
	uint8_t pressure_byte1,pressure_byte2;
	uint16_t pressure_sensor_counts;
	double pressure_sensor_pressure=0;
	PRESSURE_SENSOR_SUPPLY_PORT |= (1<< PRESSURE_SENSOR_SUPPLY_PIN);	/* Power to the pressure sensors is turned on*/
	_delay_ms(10);	// pressure sensor startup time is 3ms from power to data ready,
	
	//find which pressure sensor has been selected , set the desired pressure sensors chip select line to high
	if (sensor_id == PRESSURE_SENSOR1){
		//printf(" pressure sensor1 selected\r\n");					//  ***TEST CODE comment out on deployment
		//PRESSURE_SENSOR1_DDR |= (1<<PRESSURE_SENSOR1_PIN);
		PRESSURE_SENSOR1_PORT &= ~(1<<PRESSURE_SENSOR1_PIN); // set chip select to low to activate SPI
		//_delay_ms(3000); // this is just here for testing
	}
	else if (sensor_id == PRESSURE_SENSOR2){
		//printf("sensor2\r\n");					//  ***TEST CODE comment out on deployment
		PRESSURE_SENSOR2_PORT &= ~(1<<PRESSURE_SENSOR2_PIN); // set chip select to low to activate SPI
	}
	else if (sensor_id == PRESSURE_SENSOR3){
		//printf("sensor3\r\n");					//  ***TEST CODE comment out on deployment
		PRESSURE_SENSOR3_PORT &= ~(1<<PRESSURE_SENSOR3_PIN); // set chip select to low to activate SPI
	}
	else{
		printf("no valid pressure sensor number selected in ReadPressure()\r\n");					//  ***TEST CODE comment out on deployment
	}


	//get first byte from pressure sensor
	pressure_byte1 = (SPI_tranceiver_byte (0xFF));
	// 	printf("pressure_byte1 = ");                //  ***TEST CODE comment out on deployment
	// 	uart_printBinaryByte(0,pressure_byte1);		// Print the value from the SPI data register
	// 	printf("\r\n");	//get second byte from pressure sensor
	pressure_byte2 = (SPI_tranceiver_byte (0xFF));
	// 	printf("pressure_byte2 = ");                          //  ***TEST CODE comment out on deployment
	// 	uart_printBinaryByte(0,pressure_byte2);
	// 	printf("\r\n"
	
	//get third byte from pressure sensor
	SPI_tranceiver_byte (0xFF);
	// 	printf("pressure_byte3 = ");                          //  ***TEST CODE comment out on deployment
	// 	uart_printBinaryByte(0,pressure_byte3);
	// 	printf("\r\n");
	//get fourth byte from pressure sensor
	SPI_tranceiver_byte (0xFF);
	// 	printf("pressure_byte4 = ");                          //  ***TEST CODE comment out on deployment
	// 	uart_printBinaryByte(0,pressure_byte4);
	// 	printf("\r\n");
	// read the status bits
	switch(pressure_byte1 & 0b11000000){
		case 00 :
		//printf("Honeywell Pressure sensor: normal operation, valid data\r\n");
		// if data is valid: calculate pressure
		// get the pressure sensor counts by concatenating pressure_byte1 and pressure_byte2 and removing the status bits (2MSBs)
		pressure_sensor_counts = (((pressure_byte1 & 0b00111111)<< 8) + pressure_byte2);
		// 			printf("pressure data (counts) = 0x %x\r\n", pressure_sensor_counts);                      
		
		// calculate pressure from counts - to prevent overflow this formula had been divided to keep the data within the AVR float range. the formula is from the honneywell technical note"SPI communication with honeywell digital output pressure sensors"
		pressure_sensor_pressure= (((((pressure_sensor_counts - 0)/(163.83))*(PRESSURE_SENSOR_pressureMAX - PRESSURE_SENSOR_pressureMIN))/100)+ PRESSURE_SENSOR_pressureMIN);
		// print pressure
		//printf ("pressure in fuction: %3.2f psi\r\n", pressure_sensor_pressure);
		break;
		case 01 :
		printf("Honeywell Pressure sensor: device in command mode\r\n");
		break;
		case 10 :
		printf("Honeywell Pressure sensor: stale data - data already fetched\r\n");
		break;
		case 11 :
		printf("Honeywell Pressure sensor: diagnostic condition\r\n");
		break;
	}
	PRESSURE_SENSOR1_PORT |= (1<<PRESSURE_SENSOR1_PIN); // deselects the chip select pin attached to the pressure sensors
	PRESSURE_SENSOR2_PORT |= (1<<PRESSURE_SENSOR2_PIN); // deselects the chip select pin attached to the pressure sensors
	PRESSURE_SENSOR3_PORT |= (1<<PRESSURE_SENSOR3_PIN); // deselects the chip select pin attached to the pressure sensors
	PRESSURE_SENSOR_SUPPLY_PORT &= ~(1<< PRESSURE_SENSOR_SUPPLY_PIN);	/* Power to the pressure sensors is turned off*/
	//printf("ReadPressure() complete: Pressure sensors deselected\r\n");
	return pressure_sensor_pressure;
}
