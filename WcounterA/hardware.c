/*
 * hardware.c
 *
 * Created: 14.11.2016 10:54:52
 *  Author: Artp
 */ 

#include <avr/io.h>

#include "../common.h"

void spi_init(void){

	// cs for W5100
	CS_PORT_W51 |= (1<<CS_BIT_W51);									// pull CS pin high
	CS_DDR_W51 |= (1<<CS_BIT_W51);									// now make it an output
	// cs for SD
	CS_PORT_SD |= (1<<CS_BIT_SD);									// pull CS pin high
	CS_DDR_SD |= (1<<CS_BIT_SD);									// now make it an output

	/*настройка портов ввода-вывода все выводы, кроме MISO выходы*/

	SPI_DDR |= (1<<SPI_MOSI) | (1<<SPI_SCK);		// set MOSI, SCK  as output, others as input
//	SPI_DDR &= ~(1<<SPI_MISO);
	SPCR = (1<<SPE)|(1<<MSTR);								// enable SPI, master mode 0
	SPSR |= (1<<SPI2X);										// set the clock rate fck/2


}
