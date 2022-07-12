/*
 * uart.h
 *
 * Created: 21/05/2020 3:48:10 PM
 *  Author: cra24x & Middy
 */ 

#ifndef UART_H_
#define UART_H_

#define F_CPU 8000000UL
#define BAUD 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD * 16UL))) - 1)

#include <stdio.h>

void init_UART(int uart_port);
void init_interrupts(void);
void uart_putc (int uart_port,char send);
void usart_pstr(char *s);
int usart_putchar_printf(char var, FILE *stream);
void uart_puts (int uart_port, const char *send);
void uart_transmitByte(int uart_port,uint8_t data);
void uart_printBinaryByte(int uart_port,uint8_t byte);
char USART0_getchar(void);
void USART0_send( unsigned char data);
void UART0_getStr(char string0[], uint8_t len);

#endif /* UART_H_ */