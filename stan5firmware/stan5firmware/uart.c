/*
 * uart.c
 *
 * Created: 12/09/2020 7:50:45 PM
 *  Author: cra24x & Middy
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"


/*****************************************************/
/*		UART Functions	*/

static FILE mystdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);   //printf function

void init_UART(int uart_port)//UART init
{
	switch (uart_port)
	{
		case 0 :
		cli();
		UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   // Enable tx and Rx interrupt
		UCSR0B |= (1 << RXCIE0 );// Enable the UART Receive interrupt
		UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);// 8 bit char sizes
		UCSR0C = ((0<<USBS0)|(1 << UCSZ01)|(1<<UCSZ00));  // Set frame format: 8data, 1 stop bit. See Table 22-7 for details
		UBRR0H = (BAUD_PRESCALE >> 8);// Set baud rate
		UBRR0L = BAUD_PRESCALE;
		sei();// Globally enable interrupts
		break;
		case 1 :
		cli();
		UCSR1B |= (1 << RXEN1) | (1 << TXEN1);   // Enable uart tx and rx
		UCSR1B |= (1 << RXCIE1 );// Enable the UART Receive interrupt
		UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);// 8 bit char sizes
		UCSR1C = ((0<<USBS1)|(1 << UCSZ11)|(1<<UCSZ10));  // Set frame format: 8data, 1 stop bit. See Table 22-7 for details
		UBRR1H = (BAUD_PRESCALE >> 8);// Set baud rate
		UBRR1L = BAUD_PRESCALE;
		sei();// Globally enable interrupts
		break;
		default:
		uart_puts(0, "invalid uart_port in init_usat(), on port 1 or 2 possible, more code needed for 3-4\r\n");
	}
	
	stdout = &mystdout;   // required for printf to go to UART
	
}

// --- USRT functions --- //
void uart_putc(int uart_port,char send) // part of the printf stream
{
	switch (uart_port)
	{
		case 0 :
		// Do nothing for a bit if there is already
		// data waiting in the hardware to be sent
		while ((UCSR0A & (1 << UDRE0)) == 0) {};
		UDR0 = send;
		break;
		case 1 :
		// Do nothing for a bit if there is already
		// data waiting in the hardware to be sent
		while ((UCSR1A & (1 << UDRE1)) == 0) {};
		UDR1 = send;
		break;
		default:
		uart_puts(0, "invalid uart_port\r\n");
	}

}

void usart_pstr(char *s) {
	while (*s) {
		uart_putc(0,*s);
		s++;
	}
}

int usart_putchar_printf(char var, FILE *stream) {   // part of the printf stream

	if (var == '\n')
	uart_putc(0,'\r');
	uart_putc(0,var);
	return 0;
}




void uart_puts (int uart_port,const char *send)
{
	// Cycle through each character individually
	while (*send)
	{
		uart_putc(uart_port, *send++);
	}
}


void uart_transmitByte(int uart_port,uint8_t data)
{
	switch (uart_port)
	{
		case 0 :
		/* Wait for empty transmit buffer */
		loop_until_bit_is_set(UCSR0A, UDRE0);
		UDR0 = data;// send data
		break;
		case 1 :
		/* Wait for empty transmit buffer */
		loop_until_bit_is_set(UCSR1A, UDRE1);
		UDR1 = data;// send data
		break;
		default:
		uart_puts(0,"invalid uart_port\r\n");
	}
}

void uart_printBinaryByte(int uart_port,uint8_t byte)
{
	/* Prints out a byte as a series of 1's and 0's */
	uint8_t bit;
	for (bit = 7; bit < 255; bit--)
	{
		if (bit_is_set(byte, bit))
		uart_transmitByte(0,'1');
		else
		uart_transmitByte(0,'0');
	}
}


// ---- Middy's code ----
char USART0_getchar(void){
	cli();
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
	sei();
}

void USART0_send( unsigned char data){
	
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
	
}

void UART0_getStr(char string0[], uint8_t len){
	uint8_t i = 0;
	
	while (i<len){
		string0[i] = USART0_getchar();
		if(string0[i] == '\r'){
			string0[i] = 0;
			break;
		}
		i++;
	}
}
// ----