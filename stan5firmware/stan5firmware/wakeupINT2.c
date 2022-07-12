/*
 * wakeupINT2.c
 *
 * Created: 7/02/2019 4:59:23 PM
 *  Author: Middy
 * This code wakes up the AVR in sleep mode using pin INT2 interrupt.
 * After wakeup, the AVR disables INT2 and triggers UART1(TX1/RX1) allowing communication between AVR and SoundTrap.
 * Before going back to sleep, AVR disables UART1 and triggers INT2. (set in Main())  
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>
#include "wakeupINT2.h"
#include "pulseflow.h"

extern void uart_puts (int uart_port,const char *send);
extern void init_UART(int uart_port);

bool PCINT8flag = false;

void INT2_init(void)
{
	cli();							// Disable global interrupts
	
	DDRD &= ~(1 << PD2);			// Set PD2 (INT2) as input
	PORTD |= (1 << PD2);			// Prevent input floating by a pull-up resistor
	
	EICRA |= (1<<ISC21);
	EICRA |= (1<<ISC20);			// This 2 lines set rising edge interrupt
	
	EIMSK &= ~(1 << INT2);			// Disable INT2
	EIFR |= (1 << INT2);			// Clear any interrupt waiting on INT2
	EIMSK |= (1 << INT2);			// Enable interrupt 2 (INT2)
	
	sei();							// Enable global interrupt
}

void USART1_disable(void){	
	uart_puts(0,"USART1 disabled. INT2 triggered.\r\n");
	// Disable USART1
	UCSR1B &= ~(1<<RXEN1);
	UCSR1B &= ~(1<<TXEN1);
	UCSR1B &= ~(1<<RXCIE1);
	
	_delay_ms(1000);
	INT2_init();						// Enable interrupt 2 (INT2)
}

void USART1_re_enable(void){
	//-----Turn off INT2 interrupt
	EIMSK &= ~(1<<INT2);			// Disable interrupt 2 (INT2)
		
	//-----Turn on UART1
	init_UART(1);		//USART1 init, this is used for SoundTrap commuication via TX1/RX1
	uart_puts(0,"USART1 triggered. INT2 disabled.\r\n"); 
}

volatile uint8_t porte_prev;		// state of PORTE at last PCINT1_vect execution
volatile uint8_t portj_prev;		// state of PORTJ at last PCINT1_vect execution
// Counters on header H5 pins 2, 4 and 6
extern counter_t counter_11;
extern counter_t counter_12;
extern counter_t counter_13;

// Pin change interrupt PCINT8 and PCINT11..12 to wakeup AVR from PC terminal through RX0 and count edges on PJ2 and PJ3
void pin_change_1_init(void)
{
	cli();							// Disable global interrupts
	PCMSK1 &= ~((1 << PCINT8) | (1 << PCINT11) | (1 << PCINT12)); // Disable PCINT interrupts
	
	// set PORT E...
	DDRE &= ~(1 << PE0);			// Set PE0 (INT8) as input
	PORTE |= (1 << PE0);			// Prevent input floating by a pull-up resistor
	
	// set PORT J...
	DDRJ &= ~((1 << PJ2) | (1 << PJ3)); // Set PJ2..3 (PCINT11..12) as inputs (for use as edge counters)
	PORTJ |=  (1 << PJ2) | (1 << PJ3);  // Set pull-ups
	
	PCIFR |= (1 << PCIF1);			// Pin change interrupt flag: Clear any interrupt waiting on PCINT15:8
	PCICR |= (1 << PCIE1);			// Pin change interrupt control: Enable a range of PCINT15:8 interrupt
	PCMSK1 = (1 << PCINT8) | (1 << PCINT11) | (1 << PCINT12) ; // Enable individual interrupts
	
	porte_prev = PINE & (0x01);
	portj_prev = PINJ & (0x3F);
	sei();							// Enable global interrupt
}

/**
 *	Interrupt on pin change on PCINT pins 8..15. 
 */
ISR(PCINT1_vect) 
 {
	// check for pins that have toggled since last PCINT1 ISR execution
	uint8_t porte = PINE & (0x01);		// mask of all PCINT1 pins in PORT E
	uint8_t portj = PINJ & (0x3F);		// mask of all PCINT1 pins in PORT J
	
	uint8_t tog_e = porte ^ porte_prev;
	porte_prev = porte;
	
	uint8_t tog_j = portj ^ portj_prev;
	portj_prev = portj;
	
	if(tog_e){
		// This interrupt wakes up the ATmega2560 on RX0
		PCMSK1 &= ~(1 << PCINT8);		// Disable PCINT8 to prevent it from trapping into interrupt
		PCINT8flag = true;				// Set flag for synchronous code
	}
	if(tog_j & (1<<PJ2)){
		// count changes on PCINT11
		counter_11.pcint_counter++;
	}
	if(tog_j & (1<<PJ3)){
		// count changes on PCINT12
		counter_12.pcint_counter++;
	}
	if(tog_j & (1<<PJ4)){
		counter_13.pcint_counter++;
	}
}


