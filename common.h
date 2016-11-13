/*
 * global.h
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
#define SPI_MOSI   0x03
#define SPI_MISO   0x04
#define SPI_SCK    0x05

// W5100
#define W51_SOFTRESET 1
#define W51_NO_SPIINIT 1
#define W51_DEBUG 1

#define CS_PORT_W51		PORTB			/* target-specific port used as chip-select */
#define CS_DDR_W51		DDRB				/* target-specific DDR for chip-select */
#define CS_BIT_W51		0x02				/* target-specific port line used as chip-select */

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
unsigned char				buf[MAX_BUF];

// MQTT
#define NET_DEBUG 1

#endif /* COMMON_H_ */