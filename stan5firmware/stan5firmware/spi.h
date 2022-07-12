/*
 * spi.h
 *
 * Created: 12/09/2020 8:24:22 PM
 *  Author: cra24x
 */ 


#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

void spi_init_master (void);
uint8_t SPI_tranceiver_byte (uint8_t SPI_byte_to_send);

#endif /* SPI_H_ */