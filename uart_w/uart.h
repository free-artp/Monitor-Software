/*
 * uart.h
 *
 * Created: 14.11.2016 12:23:34
 *  Author: Artp
 */ 


#ifndef UART_H_
#define UART_H_

int uart_putchar(char c, FILE *stream);
int uart_getchar( FILE *stream);
int uart_ready();
void uart_init();

extern FILE console;

#endif /* UART_H_ */