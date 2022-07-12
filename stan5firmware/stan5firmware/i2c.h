// Functions for i2c communication

#ifndef _I2C_MOD_H_
#define _I2C_MOD_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#define I2C_MODE_BLOCKING (0)
#define I2C_MODE_TIMEOUT  (1)

// this works out to about 1.1 seconds at 8MHz...
#define I2C_TIMEOUT_1SEC  (0x000A0000)

#define I2C_TIMEOUT_100MS (0x00010000) // havne't verified this

#define I2C_TIMEOUT_1MS (0x0000028f) // havne't verified this

void initI2C(void);
    /* Sets pullups and initializes bus speed to 100kHz (at FCPU=8MHz) */

void i2cSetTimeout(uint8_t mode, uint32_t cycles);

bool i2cGotTimeout(void);

void i2cWaitForComplete(void);
                       /* Waits until the hardware sets the TWINT flag */

void i2cStart(void);
                               /* Sends a start condition (sets TWSTA) */
void i2cStop(void);
                                

void i2cSend(uint8_t data);
                   /* Loads data, sends it out, waiting for completion */

uint8_t i2cReadAck(void);
              /* Read in from slave, sending ACK when done (sets TWEA) */
uint8_t i2cReadNoAck(void);
              /* Read in from slave, sending NOACK when done (no TWEA) */


#endif
