/*
 * WcounterA.c
 *
 * Created: 11.11.2016 18:13:13
 *  Author: Artp
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

#include "hardware.h"
#include "mqtt.h"
#include "../uart_w/uart.h"
#include "../netw5100/netw5100.h"

W5100_CFG	iface =
{
	{0x00,0x16,0x36,0xDE,0x58,0xF6},			// mac_addr
	{192,168,1,233},							// ip_addr
	{255,255,255,0},							// sub_mask
	{192,168,1,1}							// gtw_addr
};


int main(void)
{
	unsigned char c;			// console command byte
	unsigned int operate;	// flag for main cycle

	operate = 1;

	clock_init();
	uart_init();

	
	printf_P( PSTR("Startup 1") );
	
	//	setup_ADC();

	//	fs_init();			// сделает и инициализацию SPI
	spi_init();
	
	printf_P( PSTR(".2") );
	
	ifconfig( &iface );

	printf_P( PSTR(".3") );

//	DisconnectSocket(0);
//	CloseSocket(0);

	printf_P( PSTR(".4") );

	sei();
	printf_P( PSTR(".5\n") );
	
	while(1)
	{
		_delay_ms(100);
		
		if (uart_ready()) {
			c = getchar();
			switch (c) {
				case 'c':
//				DisconnectSocket(usocket);
//				CloseSocket(usocket);
				operate = 0;
				break;
				case 'r':
				operate = 1;
				break;
				case 'l':
				//					fcat();
				break;
			}
		}
		
		if (!operate) break;
		mqtt_exec();		
	}
}