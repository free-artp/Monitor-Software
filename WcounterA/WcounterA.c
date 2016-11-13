/*
 * WcounterA.c
 *
 * Created: 11.11.2016 18:13:13
 *  Author: Artp
 */ 


#include <avr/io.h>

#include "mqtt.h"

int main(void)
{
	unsigned char usocket;	// socket for umqtt
	unsigned char f_uconn;	// mqtt flag about SUBSCRIBE, after CONNECT
	char msg[6];				// mqtt message buffer
	unsigned char c;			// console command byte
	unsigned int rsize;		// received bytes counter
	unsigned int operate;	// flag for main cycle
	unsigned int cnt;		// just global counter

	struct timer timer_umqtt_kepalive;
	struct timer timer_umqtt_publish;
	
	operate = 1;

	clock_init();
	
	init_uart();
	stdin = stdout = &mystdout;
	printf_P( PSTR("Startup 1") );
	
	//	setup_ADC();

	//	fs_init();			// сделает и инициализацию SPI
	//	init_spi();
	
	printf_P( PSTR(".2") );
	
	ifconfig( &iface );
	usocket = 0;
	DisconnectSocket(usocket);
	CloseSocket(usocket);
	printf_P( PSTR(".3") );

	sei();
	printf_P( PSTR(".4\n") );
	
	while(1)
	{
		_delay_ms(100);
		
		if (uart_ready()) {
			c = getchar();
			switch (c) {
				case 'c':
				DisconnectSocket(usocket);
				CloseSocket(usocket);
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