/*
 * fram.c
 *
 * Created: 9/01/2019 12:29:06 PM
 * Author: overc & pascal craw
 *
 */ 



#include "fram.h"
#include <stdio.h>
#include <util/delay.h>
#include "logging.h"
#include "uart.h"

#define FRAM_MAXADDR 32768  // the max address for the FRAM chip is 32768 bytes

/* Initialise FRAM. Set read/write addresses to initial value. 
 */
void initFRAM(){
	#ifdef LOGGING_ENABLE
	LogDebug("initFRAM() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	uart_puts(0,"initFRAM() started\r\n");
	#endif
	fram_update_seqential_address_from_fram(); //Get next available address and store to global variable on avr sequential address
	read_address = 0x02;  // sets the read start address to the beginning
	fram_memory_report(); // logs the last memory address at [info] level and print f
}

/* Write a single byte of data (8 bits) to a 16 bit address. */
void fram_writeData(uint16_t address, uint8_t value){
	i2cStart(); // open i2c comms
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );   // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	i2cSend(address >> 8);  // send memory address  high byte 
	i2cSend(address & 0xFF); // send memory address low byte 
	i2cSend(value); // 1 btye (char)send data
	i2cStop(); //i2c stop
}

/* Read a single byte of data (8 bits) from a 16 bit address */
uint8_t fram_readData(uint16_t address){
	i2cStart();
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );  // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	i2cSend(address >> 8); //address low byte
	i2cSend(address & 0xFF); //address high byte
	i2cStart(); 
	i2cSend(0b10100001 | (A0 << 1) | (A1 << 2) | (A2 << 3) );
	uint8_t value = i2cReadNoAck();
	i2cStop();
	return value;
}

/* Write a string to memory. Increments the sequential address 
 * after each string. The sequential address is saved in the memory
 * itself. If the memory address is the last address, it is careful to avoid the 
 * inbuilt overflow condition and sets the next memory address to 0x02 instead of 0x00 and 0x01
 * which are the addresses used for storing last written memory address.
 */
void fram_writeString(char * data){	
	uint8_t reached_end = 0;
	fram_update_seqential_address_from_fram(); // gets the next available memory address from the Fram via i2c and puts it into the avr variable sequential_address
	//sequential_address = 0x02; // debugging to reset fram countern only use once the comment out otherwise memory will continually write over itself
	i2cStart(); // write to the FRAM ic
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );   // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	i2cSend(sequential_address >> 8); 
	i2cSend(sequential_address & 0xFF);
	while(*data){
		if(sequential_address == FRAM_MAXADDR){  // if the fram is at the last memory address (aka full)
			sequential_address = 0x02; // start writing at fram memory location 0x0002
			printf("\r\n *** Memory overflow! *** \r\n\r\n");			
			//_delay_ms(1000);
			reached_end = 1;
			break;
		} else {
			sequential_address++;    //move to the next memory location
		}
		i2cSend(*data++); //send the byte to the next memory locations
	}
	i2cStop();
	fram_write_sequential_address_to_fram();
	if(reached_end){
		fram_writeString("\aOVERFLOW\n");	// the asci special character or escape char \a (alert/bell 0x7) is placed at the first location in the first memory location so when the memory is read we know if it has over flowed
	}	
}



/* -- Not preferred function -- 
 * Read a line of characters as delimitered by '\r\n'.
 * Only reads up to the last sequential address as saved in
 * writeString.*/
// uint8_t fram_readLine(char * line){
// 	i2cStart();
// 	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );
// 	i2cSend(read_address >> 8);
// 	i2cSend(read_address & 0xFF);
// 	i2cStart();
// 	i2cSend(0b10100001 | (A0 << 1) | (A1 << 2) | (A2 << 3) );
// 	char read_char;
// 	while(read_char != '\n'){
// 		read_char = i2cReadAck();
// 		read_address++;
// 		if(read_address > sequential_address){
// 			*line = 0;
// 			read_char = i2cReadNoAck();
// 			i2cStop();
// 			return 0;
// 		} else {
// 			(*line++) = read_char;
// 		}
// 	}
// 	read_char = i2cReadNoAck();
// 	// terminating character!
// 	(*line++) = 0;
// 	i2cStop();
// 	return 1;
// }

/* -- Not preferred function --
 * Read all lines in one function call*/
// void fram_readLines(){
// 	int line_count = 0;
// 	read_address = 0x02;
// 	int memory_to_read = 1;
// 	char line[64];
// 	while(memory_to_read){
// 		memory_to_read = fram_readLine(&line);
// 		line_count++;
// 		printf(line);
// 	}
// 	printf("Printed %d lines\n", line_count);
// }

/* Read every byte of memory into printf().
 * Read up to the last written address, or every address if there was a memory overflow 
 */
void fram_readAll(){
	printf("----- fram_readAll() started -----\r\n");
	read_address = 0x0002; // read from the beginning of the memory
	uint16_t read_upto_address_locations = sequential_address; // make a new variable the same as sequential address which can be changed if an overflow has occurred
	char read_char; // char to hold what is read from FRAM memory
	// start reading from the begining
	i2cStart();
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );   // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	i2cSend(read_address >> 8);
	i2cSend(read_address & 0xFF);
	i2cStart();
	i2cSend(0b10100001 | (A0 << 1) | (A1 << 2) | (A2 << 3) );   // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	// check the first character to see if overflow has previously occured
	read_char = i2cReadAck(); // read the first char from fram i2c
	printf("%c", read_char); // print the first char to printf
	if (read_char == 7){ // if the ASCII alert/bell escape char is seen this indicates an overflow event has occured
		read_upto_address_locations = FRAM_MAXADDR;
		printf("\r\n *** Overflow detected from previous runs, will print all memory ***\r\n\r\n");
		_delay_ms(1000); // 1 second delay to give the user time to see the overflow note
	}
	read_address++;	// increment the read address to the next 16bit memory location
	while(read_address < (read_upto_address_locations - 1)){  // while not at the last written memory address OR 
		read_char = i2cReadAck();
		printf("%c", read_char);
		read_address++;
	}
	read_char = i2cReadNoAck();
	printf("%c", read_char);
	i2cStop();
	printf("----- fram_readAll() complete -----\r\n");
}



/* Used in fram_writeString. Write the last saved address in memory
 * so we know where to save the next characters to memory. */
void fram_write_sequential_address_to_fram(){
	//printf("sequential address: %s", "sequential_address");
	//sequential_address = 256;  // this is put in for testing to force what is written to the fram
	i2cStart();
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) );   // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	i2cSend(0x00 >> 8);
	i2cSend(0x00 & 0xFF);
	i2cSend(sequential_address >> 8);
	i2cSend(sequential_address & 0xFF);
	i2cStop();
}

/* Reset write address.
 * Sets the write address to the beginning to start writing over again.
 * Quicker, simpler and reduces wear vs wiping all the whole memory.
 */
void fram_resetWriteAddress(){
	#ifdef LOGGING_ENABLE
		LogDebug("fram_resetWriteAddress() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		uart_puts(0,"fram_resetWriteAddress() started\r\n");
	#endif
		sequential_address = 0x02;
		fram_write_sequential_address_to_fram();
	
	#ifdef LOGGING_ENABLE
		LogVerbose("fram_resetWriteAddress() complete\r\n");
	#endif
}

/* Reset the read address. Only needed for readLine calls.*/
void fram_resetReadAddress(){
	read_address = 0x02;
}

// check how much memory is used and log to serial and fram
void fram_memory_report(){
	#ifdef LOGGING_ENABLE
		LogDebug("fram_memory_report() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		uart_puts(0,"fram_memory_report() started\r\n");
	#endif
	// make a string with the current sequential write address and the max address
	char mem_str[80];
	sprintf(mem_str,"FRAM sequential Address:%i/32768 bytes\r\n",fram_return_sequential_address());
	// print the string
	#ifdef LOGGING_ENABLE
		LogInfo(mem_str);
		LogVerbose("fram_memory_report() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
		uart_puts(0,mem_str);
	#endif
}

uint16_t fram_return_sequential_address(){ // reads and returns the next available address for writing to the fram chip, this 16bit memory address is stored in the first two 8bit memory locatipons on the fram ic 
	uint16_t last_address = 0x0000; //variable to hold the 16bit address from the first two 8 bit memory locations
	//Get last written address in location
	i2cStart();			// start I2c communications
	i2cSend(0b10100000 | (A0 << 1) | (A1 << 2) | (A2 << 3) ); // this is the device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	//send 16 bit address of first fram sequential address memory location x0000
	i2cSend(0x00 >> 8);   // fram sequential address high byte
	i2cSend(0x00 & 0xFF); // fram sequential address low byte
	i2cStart(); //
	i2cSend(0b10100001 | (A0 << 1) | (A1 << 2) | (A2 << 3) ); // device address, 4x MSB are fixed for this device 1010,pins at A1...A2 determine LSBs 1...3, the LSB (0) determines R/W, 1=Read, 0=write
	uint8_t address_high = i2cReadAck(); // get first 8 bits of sequential address data from 16 bit data location 0x00
	uint8_t address_low = i2cReadNoAck(); // get second 8 bits of sequential address data from 16 bit data location 0x01
	i2cStop(); // end read
	last_address = (address_high << 8) | address_low;  // put the high and low data bytes into a 16bit uint16_t
	return last_address;
}

void fram_update_seqential_address_from_fram(){
	sequential_address = fram_return_sequential_address();
}