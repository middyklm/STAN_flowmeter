#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"

extern void uart_puts (int uart_port,const char *send);

uint8_t timeout_mode;
uint32_t timeout_cycles;
volatile bool lastTimeout;

void initI2C(void) {
	
	#ifdef LOGGING_ENABLE
		LogDebug("initI2C() started\r\n");
	#endif
	TWBR = 32;   /* set bit rate (p.242): 8MHz / (16+2*TWBR*1) ~= 100kHz */
	TWCR |= (1 << TWEN);                                      
	timeout_mode = 0;
	lastTimeout = false;
	#ifdef LOGGING_ENABLE
		LogDebug("initI2C() complete\r\n");
	#endif
}

void i2cSetTimeout(uint8_t mode, uint32_t cycles){
	if (mode==I2C_MODE_BLOCKING){
		timeout_mode = 0;
	}
	else{
		timeout_mode = 1;
		timeout_cycles = cycles;
	}
	lastTimeout = false;
}

inline void i2cCompleteOrTimeout();
inline void i2cCompleteOrTimeout() {
	uint32_t i;
	lastTimeout = false;
	for(i=0;i<timeout_cycles;i++){
		if (TWCR & (1<<TWINT))
			return;
	}
	lastTimeout = true;
}

bool i2cGotTimeout(void){
	return lastTimeout;
}

void i2cWaitForComplete(void) {
	if (timeout_mode){
		i2cCompleteOrTimeout();
	}
	else{
		loop_until_bit_is_set(TWCR, TWINT);
	}
}

void i2cStart(void) {
  TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTA));
  i2cWaitForComplete();
}

void i2cStop(void) {
  TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTO));
}

uint8_t i2cReadAck(void) {
  TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWEA));
  i2cWaitForComplete();
  return (TWDR);
}

uint8_t i2cReadNoAck(void) {
  TWCR = (_BV(TWINT) | _BV(TWEN));
  i2cWaitForComplete();
  return (TWDR);
}

void i2cSend(uint8_t data) {
  TWDR = data;
  TWCR = (_BV(TWINT) | _BV(TWEN));                  /* init and enable */
  i2cWaitForComplete();
}
