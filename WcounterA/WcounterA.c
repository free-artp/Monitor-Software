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
#include "adc.h"
#include "../clock/timer.h"

#define CONSOLE_DEBUG 1
//#undef CONSOLE_DEBUG

#include "debug.h"

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
	struct timer timer_adc;
	char msg[10];

	operate = 1;

	uart_init();
	debug_print_P( "uart\n" );

	clock_init();
	debug_print_P( "clock %d\n" );

	spi_init();
	debug_print_P("spi\n");

	setup_ADC();
	timer_set( &timer_adc, CLOCK_SECOND * 5 );
	debug_print_P( "adc\n" );

	//	fs_init();			// сделает и инициализацию SPI
	
	ifconfig( &iface );
	debug_print_P( "network\n" );

	sei();
	debug_print_P( "sei\n" );

	debug_print_P( "main cycle\n" );
	while(1)
	{
		_delay_ms(100);
		
		if (uart_ready()) {
			c = getchar();
			switch (c) {
				case 's':
					operate = 0;
					break;
				case 'r':
					operate = 1;
					break;
			}
		}
		
		if (!operate) break;
		
		mqtt_exec();	
		
		if ( timer_tryreset(&timer_adc) ) {
			sprintf( msg, "%u", DATA);
			mqtt_publish( mqtt_full_topic_P(PSTR("data")), (uint8_t *)msg, strlen(msg));
		}
		
	}	// while
}