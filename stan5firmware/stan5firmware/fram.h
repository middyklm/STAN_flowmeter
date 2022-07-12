/*
 * fram.h
 *
 * Created: 9/01/2019 12:21:33 PM
 *  Author: overc & pascal craw
 *
 * Original code written by Leo Febey, modified by pascal to have the current read location stored on the FRAM chip itself rather than having it stored
 * on the AVR which is wiped when reset, this allows the memory to hold data over the reset. 
 *
 * The code works by sequentially writing bytes (8 bits) into 16 bit addresses. When the end of the memory is reached an overflow occurs and the earliest written data is wiped over.  
 * A special character (0x7 ASCII bel/aleret) is writte into the first memory address to signify when an overflow has occurred. When printing out data to printf the data from the beggingin of the chip 
 * is fed out until the current write position is reached, if an overflow has occurred then the whole memory is read out to cover both the begining and end of the memory.
 */ 


#ifndef FRAM_H_
#define FRAM_H_

#include "i2c.h"
#define A0 0
#define A1 0
#define A2 0

volatile uint16_t sequential_address; // next available fram memory location for writing to, this is stored on the AVR and lost at reset
uint16_t read_address; // holds the current read address


/* Initialise FRAM. Read last written memory address*/
void initFRAM();

/* Write a single byte of data to a particular address */
void fram_writeData(uint16_t address, uint8_t value);

/* Read a single byte of data from a particular address */
uint8_t fram_readData(uint16_t address);

/* Write a sequential string of data to FRAM */
void fram_writeString(char * str);

/* Read each byte of data on the FRAM into printf().*/
void fram_readAll();

/* Internal. Write the last memory address+1 written to the first two memory addresses of the FRAM */
void fram_write_sequential_address_to_fram();

/* Clear all memory addresses to 0 */
void fram_wipeMemory();

/* Quick way to reset writing to the FRAM. */
void fram_resetWriteAddress();

/* Reset the reading address. Not needed with readAll() */
void fram_resetReadAddress();

// logs & prints the memory status to printf and fram
void fram_memory_report();

uint8_t fram_readLine(char * line);
void fram_readLines();

uint16_t fram_return_sequential_address();  // returns the current sequential write address from the fram

void fram_update_seqential_address_from_fram(); // reads the current write address and updates the variable sequential_address held on the AVR

#endif /* FRAM_H_ */