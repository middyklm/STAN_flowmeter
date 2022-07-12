/*
 * spi.c
 *
 * Created: 12/09/2020 8:24:10 PM
 *  Author: cra24x
 */ 


#include "spi.h"
#include "stdio.h"
#include <stdint.h>
#include <avr/io.h>

// SPI DEFINES
#define SPI_PORT		PORTB
#define SPI_DDR			DDRB
#define SPI_SCK_PIN		PB1
#define SPI_MOSI_PIN	PB2
#define SPI_MISO_PIN	PB3
#define SPI_SS_PIN		PB0


/*		SPI Bus Functions	*/
/*****************************************************/
// Initialize SPI Master Device (without interrupt)
void spi_init_master (void)
{
	#ifdef LOGGING_ENABLE
	LogDebug("spi_init_master() started\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("spi_init_master() started\r\n");
	#endif
	
	// Set MOSI, SCK, SS as Output SS must be output (or input held HIGH for SPI master to work)  
	SPI_DDR |= (1<<SPI_MOSI_PIN)|(1<<SPI_SCK_PIN)|(1<<SPI_SS_PIN);
	// Enable SPI, Set as Master, Prescaler: Fosc/16, Honeywell pressure sensor operates in SPI mode0 (SCLK low during idle and samples on leading clock edge)
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	#ifdef LOGGING_ENABLE
	LogDebug("spi_init_master() complete\r\n");
	#endif
	#ifndef LOGGING_ENABLE
	printf("spi_init_master() complete\r\n");
	#endif
}

uint8_t SPI_tranceiver_byte (uint8_t SPI_byte_to_send){		// sends and receives one byte from the SPI bus
	SPDR = SPI_byte_to_send; // Load data into the buffer
	//printf("SPI data register loaded with byte, waiting for transmission to complete\r\n");
	while(!(SPSR & (1<<SPIF) ));   //Wait until transmission complete
	return SPDR; // read data from SPI register
}
