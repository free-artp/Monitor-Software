/*
 * netw5100.c
 *
 * Created: 10.11.2016 19:14:10
 *  Author: Artp
 */ 
/*****************************************************************************
//  File Name    : wiznetping.c
//  Version      : 1.0
//  Description  : Wiznet W5100
//  Author       : RWB
//  Target       : AVRJazz Mega168 Board
//  Compiler     : AVR-GCC 4.3.2; avr-libc 1.6.6 (WinAVR 20090313)
//  IDE          : Atmel AVR Studio 4.17
//  Programmer   : AVRJazz Mega168 STK500 v2.0 Bootloader
//               : AVR Visual Studio 4.17, STK500 programmer
//  Last Updated : 01 July 2010
*****************************************************************************/


/*
 *  The following code turns the above wiznetping.c source code into a
 *  generic library of W5100 support routines that are target-independent.
 *  That is, you build this library for a generic AVR ATmega device, then
 *  write your application to use the W51_xxx routines below for accessing
 *  the W5100 chip.  Because these routines are target-independent, you
 *  never have to rebuild them just because you are moving your code from,
 *  say, a 'mega128 to an 'xmega128a1 device.
 *
 *  For this to work properly, your application must provide three target-
 *  specific functions and must register the addresses of those functions
 *  with the W5100 library at run-time.  These functions are:
 *
 *  select		target-specific function for enabling the W5100 chip
 *  xchg		target-specific function for exchanging a byte with the W5100 chip
 *  deselect	target-specific function for disabling the W5100 chip
 *  reset		target-specific function for hardware reset of the W5100 chip
 *
 *  Your application registers these three functions with the W5100 library
 *  by invoking the W51_register() function.  Your application must make this
 *  call one time and must make this call before calling any other W5100
 *  functions.
 */

#include <avr/io.h>

#include <util/delay.h>
#include <stdio.h>

#include "../common.h"
#include "netw5100.h"

//============================================== Low end function ====================================


/*
 *  Initialize the ATmega328p SPI subsystem
 */
#ifndef W51_NO_SPIINIT
void spi_init_w51(void){

	CS_PORT_W51 |= (1<<CS_BIT_W51);									// pull CS pin high
	CS_DDR_W51 |= (1<<CS_BIT_W51);									// now make it an output

	SPI_DDR = (1<<PORTB3) | (1<<PORTB5) | (1<<PORTB2);		// set MOSI, SCK and SS as output, others as input

	SPCR = (1<<SPE)|(1<<MSTR);								// enable SPI, master mode 0
	SPSR |= (1<<SPI2X);										// set the clock rate fck/2

}
#endif

/*
 *  Simple wrapper function for selecting the W5100 device.  This function
 *  allows the library code to invoke a target-specific function for enabling
 *  the W5100 chip.
 */
void  select(void)
{
	CS_PORT_W51&=~(1<<CS_BIT_W51);
}

/*
 *  Simple wrapper function for deselecting the W5100 device.  This function
 *  allows the library code to invoke a target-specific function for disabling
 *  the W5100 chip.
 */
void  deselect(void)
{
	CS_PORT_W51|=(1<<CS_BIT_W51);
}


/*
 *  my_xchg      callback function; exchanges a byte with W5100 chip
 */
unsigned char  xchg(unsigned char  val)
{
	SPDR = val;
	while  (!(SPSR & (1<<SPIF)))  ;
	_delay_ms(1);
	return  SPDR;
}


#ifndef W51_SOFTRESET

/*
 *  my_reset      callback function; force a hardware reset of the W5100 device
 */

void  reset(void)

	RESET_PORT_W51 |= (1<<RESET_BIT_W51);							// pull reset line high
	RESET_DDR_W51 |= (1<<RESET_BIT_W51);							// now make it an output
	RESET_PORT_W51 &= ~(1<<RESET_BIT_W51);							// pull the line low
	_delay_ms(5);											// let the device reset
	RESET_PORT_W51 |= (1<<RESET_BIT_W51);							// done with reset, pull the line high
	_delay_ms(10);											// let the chip wake up
}

#endif


/*
** ---------------------- Driver's function
*/


void  W51_write(unsigned int  addr, unsigned char  data)
{
//	printf("w %03x-%02x\n", addr, data);
	select();									// enable the W5100 chip
	xchg(W5100_WRITE_OPCODE);					// need to write a byte
	xchg((addr & 0xff00) >> 8);					// send MSB of addr
	xchg(addr & 0xff);							// send LSB
	xchg(data);									// send the data
	deselect();									// done with the chip
}


unsigned char  W51_read(unsigned int  addr)
{
	unsigned char				val;

	select();								// enable the W5100 chip
	xchg(W5100_READ_OPCODE);					// need to read a byte
	xchg((addr & 0xff00) >> 8);				// send MSB of addr
	xchg(addr & 0xff);						// send LSB
	val = xchg(0x00);						// need to send a dummy char to get response
	deselect();								// done with the chip
//	printf("r %03x-%02x\n", addr, val);
	return  val;								// tell her what she's won
}



void  W51_init(void)
{

#ifndef W51_NO_SPIINIT
	spi_init_w51();
#endif

#ifdef W51_SOFTRESET
		W51_write(W5100_MR, W5100_MR_SOFTRST); 		// otherwise, force the w5100 to soft-reset
#else
		reset();									// if host provided a reset function, use it
#endif
	_delay_ms(10);
}



unsigned char  W51_config(W5100_CFG  *pcfg)
{
	if (pcfg == 0)  return  W5100_FAIL;

	W51_write(W5100_GAR + 0, pcfg->gtw_addr[0]);		// set up the gateway address
	W51_write(W5100_GAR + 1, pcfg->gtw_addr[1]);
	W51_write(W5100_GAR + 2, pcfg->gtw_addr[2]);
	W51_write(W5100_GAR + 3, pcfg->gtw_addr[3]);
	_delay_ms(1);

	W51_write(W5100_SHAR + 0, pcfg->mac_addr[0]);	// set up the MAC address
	W51_write(W5100_SHAR + 1, pcfg->mac_addr[1]);
	W51_write(W5100_SHAR + 2, pcfg->mac_addr[2]);
	W51_write(W5100_SHAR + 3, pcfg->mac_addr[3]);
	W51_write(W5100_SHAR + 4, pcfg->mac_addr[4]);
	W51_write(W5100_SHAR + 5, pcfg->mac_addr[5]);
	_delay_ms(1);

	W51_write(W5100_SUBR + 0, pcfg->sub_mask[0]);	// set up the subnet mask
	W51_write(W5100_SUBR + 1, pcfg->sub_mask[1]);
	W51_write(W5100_SUBR + 2, pcfg->sub_mask[2]);
	W51_write(W5100_SUBR + 3, pcfg->sub_mask[3]);
	_delay_ms(1);

	W51_write(W5100_SIPR + 0, pcfg->ip_addr[0]);		// set up the source IP address
	W51_write(W5100_SIPR + 1, pcfg->ip_addr[1]);
	W51_write(W5100_SIPR + 2, pcfg->ip_addr[2]);
	W51_write(W5100_SIPR + 3, pcfg->ip_addr[3]);
	_delay_ms(1);

//	W51_write(W5100_RTR, 0x00);						// Retransmission time 100 - 10 ms
//	W51_write(W5100_RTR+1, 0x64);
	W51_write(W5100_RTR, 0x00);						// Retransmission time 200 - 20 ms
	W51_write(W5100_RTR+1, 0xC8);
//	W51_write(W5100_RTR, 0x01);						// Retransmission time 500 - 50 ms
//	W51_write(W5100_RTR+1, 0xF4);
	W51_write(W5100_RCR, 4);						// retransmission counter =4
//	W51_write(W5100_RCR, 1);						// retransmission counter =10


	W51_write(W5100_RMSR, 0x55);						// use default buffer sizes (2K bytes RX and TX for each socket
	W51_write(W5100_TMSR, 0x55);						// use default buffer sizes (2K bytes RX and TX for each socket


	return  W5100_OK;								// everything worked, show success
}


//============================================== interface ====================================
unsigned char  ifconfig( NET_CFG  *pcfg )
{
	unsigned char ret;
	ret = NET_FAIL;
	
	W51_init();	// reset
	
	ret = W51_config( (W5100_CFG*)pcfg);		// set MAC, IP, gw, netmask

	return ret;
}

unsigned char  		SocketState(unsigned char  sock)
{
	unsigned char ret;
	unsigned int	 sockaddr;
	
	sockaddr = W5100_SKT_BASE(sock);
	
	ret = W51_read(sockaddr+W5100_SR_OFFSET);
	
	return ret;
}




unsigned char  OpenSocket(unsigned char  sock, unsigned char  eth_protocol, unsigned int  tcp_port)
{
	unsigned char			retval;
	unsigned int			sockaddr;
	
	retval = W5100_FAIL;								// assume this doesn't work
	if (sock >= W5100_NUM_SOCKETS)  return retval;	// illegal socket value is bad!

	sockaddr =  W5100_SKT_BASE(sock);				// calc base addr for this socket

	if ( !(W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_CLOSED) )    // Make sure we close the socket first
	{
		CloseSocket(sock);
	}
	W51_write(sockaddr+W5100_IR_OFFSET, 0xFF);		// reset all interrupts in this socket.

	W51_write(sockaddr+W5100_MR_OFFSET ,eth_protocol);						// set protocol for this socket
//	W51_write(sockaddr+W5100_MR_OFFSET , W5100_SKT_MR_TCP);						// set protocol for this socket
	W51_write(sockaddr+W5100_PORT_OFFSET, ((tcp_port & 0xFF00) >> 8 ));		// set port for this socket (MSB)
	W51_write(sockaddr+W5100_PORT_OFFSET + 1, (tcp_port & 0x00FF));			// set port for this socket (LSB)
	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_OPEN);	               	// open the socket

	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;							// loop until device reports socket is open (blocks!!)

	if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_INIT)  retval = sock;	// if success, return socket number
	else  CloseSocket(sock);													// if failed, close socket immediately

	return  retval;
}


void  CloseSocket(unsigned char  sock)
{
	unsigned int			sockaddr;
	
	if (sock > W5100_NUM_SOCKETS)  return;					// if illegal socket number, ignore request
	sockaddr = W5100_SKT_BASE(sock);							// calc base addr for this socket

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CLOSE);	// tell chip to close the socket
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;			// loop until socket is closed (blocks!!)
}


void  DisconnectSocket(unsigned char  sock)
{
	unsigned int			sockaddr;
	
	if (sock > W5100_NUM_SOCKETS)  return;			// if illegal socket number, ignore request
	sockaddr = W5100_SKT_BASE(sock);				// calc base addr for this socket

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_DISCON);		// disconnect the socket
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;	// loop until socket is closed (blocks!!)
}


unsigned char  Connect(unsigned char  sock, unsigned char* addr, unsigned int tcp_port)
{
	unsigned char			retval;
	unsigned int			sockaddr;
	unsigned char ret;
	
	#ifdef W51_DEBUG
		unsigned char ret2, ret3;
		unsigned int	 cnt;
	#endif

	retval = W5100_FAIL;												// assume this fails
	if (sock > W5100_NUM_SOCKETS)  return  retval;					// if illegal socket number, ignore request
	sockaddr = W5100_SKT_BASE(sock);									// calc base addr for this socket
/*
	ret = W51_read(sockaddr+W5100_DIPR_OFFSET+0);
	printf("addr:%d.", ret);
	ret = W51_read(sockaddr+W5100_DIPR_OFFSET+1);
	printf("%d.", ret);
	ret = W51_read(sockaddr+W5100_DIPR_OFFSET+2);
	printf("%d.", ret);
	ret = W51_read(sockaddr+W5100_DIPR_OFFSET+3);
	printf("%d:", ret);
	ret = W51_read(sockaddr+W5100_DPORT_OFFSET);
	printf("%02x", ret);
	ret = W51_read(sockaddr+W5100_DPORT_OFFSET+1);
	printf("%02x\n", ret);
*/
	if( (ret=W51_read(sockaddr+W5100_SR_OFFSET)) == W5100_SKT_SR_INIT )		// if socket is in initialized state...
	{
//		printf("A: 0x%02x\n", ret);

		W51_write(sockaddr+W5100_DIPR_OFFSET   ,addr[0]);						// set destination IP for this socket
		W51_write(sockaddr+W5100_DIPR_OFFSET+1 ,addr[1]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+2 ,addr[2]);
		W51_write(sockaddr+W5100_DIPR_OFFSET+3 ,addr[3]);

		W51_write(sockaddr+W5100_DPORT_OFFSET, ((tcp_port & 0xFF00) >> 8 ));		// set port for this socket (MSB)
		W51_write(sockaddr+W5100_DPORT_OFFSET + 1, (tcp_port & 0x00FF));			// set port for this socket (LSB)

		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_CONNECT);		// put socket in connect state

		while ( (ret = W51_read(sockaddr+W5100_CR_OFFSET)) ) ;					// block until command is accepted
//		printf("B: 0x%02x\n", ret);
		
		#ifdef W51_DEBUG
			cnt = 0;
			ret3 = 0xFF;
			#define _w51_cnt_check (cnt<6000)
		#else
			#define _w51_cnt_check 1
		#endif
		do {
			ret = W51_read(sockaddr+W5100_SR_OFFSET);
			#ifdef W51_DEBUG
				ret2 = W51_read(sockaddr+W5100_IR_OFFSET	);
				if (ret3 != ret2)
					printf("%d:%02x-%02x ", cnt, ret, ret2);
				ret3 = ret2;
				cnt++;
			#endif
		}
		while (ret != W5100_SKT_SR_ESTABLISHED && ret != W5100_SKT_SR_CLOSED  && _w51_cnt_check );

		if ( (ret = W51_read(sockaddr+W5100_SR_OFFSET)) == W5100_SKT_SR_ESTABLISHED)  retval = W5100_OK;	// if socket state changed, show success
		else  CloseSocket(sock);					// not in established mode, close and show an error occurred

	}
	return  retval;
	
	
}

unsigned char  Listen(unsigned char  sock)
{
	unsigned char			retval;
	unsigned int			sockaddr;

	retval = W5100_FAIL;												// assume this fails
	if (sock > W5100_NUM_SOCKETS)  return  retval;					// if illegal socket number, ignore request

	sockaddr = W5100_SKT_BASE(sock);									// calc base addr for this socket
	if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_INIT)		// if socket is in initialized state...
	{
		W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_LISTEN);		// put socket in listen state
		while (W51_read(sockaddr+W5100_CR_OFFSET))  ;					// block until command is accepted

		if (W51_read(sockaddr+W5100_SR_OFFSET) == W5100_SKT_SR_LISTEN)  retval = W5100_OK;	// if socket state changed, show success
		else  CloseSocket(sock);					// not in listen mode, close and show an error occurred
	}
	return  retval;
}

unsigned int free_buff_size(unsigned char sock){
	unsigned int txsize;
	
	txsize = W51_read( W5100_SKT_BASE(sock) + W5100_TX_FSR_OFFSET);	
	txsize = (((txsize & 0x00FF) << 8 ) + W51_read(W5100_SKT_BASE(sock) + W5100_TX_FSR_OFFSET + 1));
	
	return txsize;
}


unsigned char  Send(unsigned char  sock,  unsigned char  *buf, unsigned int  buflen)
{
	unsigned int					ptr;
	unsigned int					offaddr;
	unsigned int					realaddr;
//	unsigned int					txsize;
	unsigned int					timeout;
	unsigned int					sockaddr;

	if (buflen == 0 || sock >= W5100_NUM_SOCKETS)  return  W5100_FAIL;		// ignore illegal requests
	sockaddr = W5100_SKT_BASE(sock);			// calc base addr for this socket
/*
	// Make sure the TX Free Size Register is available
	txsize = W51_read(sockaddr+W5100_TX_FSR_OFFSET);		// make sure the TX free-size reg is available
	txsize = (((txsize & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1));

	timeout = 0;
	while (txsize < buflen)
	{
		_delay_ms(1);

		txsize = W51_read(sockaddr+W5100_TX_FSR_OFFSET);		// make sure the TX free-size reg is available
		txsize = (((txsize & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_FSR_OFFSET + 1));

		if (timeout++ > 1000) 						// if max delay has passed...
		{
			DisconnectSocket(sock);					// can't connect, close it down
			return  W5100_FAIL;						// show failure
		}
	}
*/
	// check the free size in w5100 buffer
	timeout = 0;
	while ( free_buff_size(sock) < buflen) {
		_delay_ms(1);
		if ( timeout++ > 1000 ) {
			DisconnectSocket(sock);
			return W5100_FAIL;
		}
	}

	// Read the Tx Write Pointer
	ptr = W51_read(sockaddr+W5100_TX_WR_OFFSET);
	offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_TX_WR_OFFSET + 1));

	while (buflen)
	{
		buflen--;
		realaddr = W5100_TXBUFADDR + (offaddr & W5100_TX_BUF_MASK);		// calc W5100 physical buffer addr for this socket

		W51_write(realaddr, *buf);					// send a byte of application data to TX buffer
//		printf("0x%02x\n", *buf);
		offaddr++;									// next TX buffer addr
		buf++;										// next input buffer addr
	}

	W51_write(sockaddr+W5100_TX_WR_OFFSET, (offaddr & 0xFF00) >> 8);	// send MSB of new write-pointer addr
	W51_write(sockaddr+W5100_TX_WR_OFFSET + 1, (offaddr & 0x00FF));		// send LSB

	W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_SEND);	// start the send on its way
	while (W51_read(sockaddr+W5100_CR_OFFSET))  ;	// loop until socket starts the send (blocks!!)

	return  W5100_OK;
}





unsigned int  Receive(unsigned char  sock, unsigned char  *buf, unsigned int  buflen)
{
    unsigned int					ptr;
	unsigned int					offaddr;
	unsigned int					realaddr;   	
	unsigned int					sockaddr;

    if (buflen == 0 || sock >= W5100_NUM_SOCKETS)  return  W5100_FAIL;		// ignore illegal conditions

	if (buflen > (MAX_BUF-2))  buflen = MAX_BUF - 2;		// requests that exceed the max are truncated

	sockaddr = W5100_SKT_BASE(sock);						// calc base addr for this socket
	ptr = W51_read(sockaddr+W5100_RX_RD_OFFSET);			// get the RX read pointer (MSB)
    offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(sockaddr+W5100_RX_RD_OFFSET + 1));		// get LSB and calc offset addr

	while (buflen)
	{
		buflen--;
		realaddr = W5100_RXBUFADDR + (offaddr & W5100_RX_BUF_MASK);
		*buf = W51_read(realaddr);
		offaddr++;
		buf++;
	}
	*buf='\0'; 												// buffer read is complete, terminate the string

    // Increase the S0_RX_RD value, so it point to the next receive
	W51_write(sockaddr+W5100_RX_RD_OFFSET, (offaddr & 0xFF00) >> 8);	// update RX read offset (MSB)
    W51_write(sockaddr+W5100_RX_RD_OFFSET + 1,(offaddr & 0x00FF));		// update LSB

    // Now Send the RECV command
    W51_write(sockaddr+W5100_CR_OFFSET, W5100_SKT_CR_RECV);			// issue the receive command
    _delay_us(5);											// wait for receive to start

    return  W5100_OK;
}


unsigned int  ReceivedSize(unsigned char  sock)
{
	unsigned int					val;
	unsigned int					sockaddr;

	if (sock >= W5100_NUM_SOCKETS)  return  0;
	sockaddr = W5100_SKT_BASE(sock);						// calc base addr for this socket
	val = W51_read(sockaddr+W5100_RX_RSR_OFFSET) & 0xff;
	val = (val << 8) + W51_read(sockaddr+W5100_RX_RSR_OFFSET + 1);
	return  val;
}



