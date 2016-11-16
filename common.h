/*
 * common.h
 *
 * Created: 07.10.2016 12:13:16
 *  Author: Artp
 */ 


#ifndef COMMON_H_
#define COMMON_H_

/*
 *  Define the SPI port, used to exchange data with a W5100 chip.
 *	 Atmega328p
 *	SCK		- PB5
 *	MISO		- PB4
 *	MOSI		- PB3
 *	SS		- PB2
 *	
 */

#define SPI_PORT 	PORTB			/* target-specific port containing the SPI lines */
#define SPI_DDR  	DDRB				/* target-specific DDR for the SPI port lines */
#define SPI_MOSI   PORTB3
#define SPI_MISO   PORTB4
#define SPI_SCK    PORTB5


// W5100
#define W51_SOFTRESET 1
#define W51_NO_SPI_INIT 1

#define CS_PORT_W51		PORTB			/* target-specific port used as chip-select */
#define CS_DDR_W51		DDRB				/* target-specific DDR for chip-select */
#define CS_BIT_W51		PORTB2

#ifndef W51_SOFTRESET
	#define RESET_DDR_W51   DDRD				/* target-specific DDR for reset */
	#define RESET_PORT_W51	PORTD			/* target-specific port used for reset */
	#define RESET_BIT_W51	0x03				/* target-specific port line used as reset */
#endif

// SD-card
#define CS_PORT_SD		PORTB			/* target-specific port used as chip-select */
#define CS_DDR_SD		DDRB				/* target-specific DDR for chip-select */
#define CS_BIT_SD		0x00				/* target-specific port line used as chip-select */

// Common (Net51 and fs) buffer
#define  MAX_BUF		512			/* largest buffer we can read from chip */
extern unsigned char				buf[MAX_BUF];

//---------------------------- DEBUG ---------------------------

// W5100 debugging
#define W51_DEBUG 1
//#define W51_DEBUG_SPI 1				/* печать всего, что шлем или получаем по SPI в  W5100 */

// MQTT
#define MQTT_DEBUG 1						/* печать буффера mqtt перед посылкой и после получения */

#endif /* COMMON_H_ */