/*
 * uart_w.c
 *
 * Created: 14.11.2016 12:21:59
 *  Author: Artp
 */ 

#include <avr/io.h>
#include <stdio.h>

#define BAUD 57600
#include <util/setbaud.h>

#include "uart.h"

FILE console = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int uart_putchar(char c, FILE *stream)
{
	if (c == '\n')
	uart_putchar('\r', stream);
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar( FILE *stream)
{
	char c;
	while ( !(UCSR0A & (1<<RXC0)) ) {};
	c = UDR0;
	uart_putchar(c, stream);
	return c;
}

int uart_ready(void)
{
	return (UCSR0A & (1<<RXC0));
}

void uart_init() {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */

	stdin = stdout = &console;
}


