/*
 * network.c
 *
 * Created: 11.11.2016 18:14:04
 *  Author: Artp
 */ 

#include <avr/pgmspace.h>

#include <util/delay.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../common.h"

#include "../umqtt/umqtt.h"
#include "../netw5100/netw5100.h"
#include "mqtt.h"

unsigned char	 buf[MAX_BUF];
static unsigned int cnt = 0;

#define UBUFFS 150
uint8_t u_rxbuff[UBUFFS];
uint8_t u_txbuff[UBUFFS];


#ifdef MQTT_DEBUG
void buffer_print( unsigned char *buf, unsigned int len) {
	unsigned int i;
	for(i=0; i<len; i++) printf("0x%02x ", buf[i]);
	printf("\n");
}
#endif


void mqtt_message_callback(struct umqtt_connection *uc, char *topic, uint8_t *data, int len)
{
	printf("%s %s\n", topic, data);
}

void mqtt_connected_callback(struct umqtt_connection *u_conn){
	struct mqtt_connection *m;
	m = (struct umqtt_connection *)u_conn->private;
	umqtt_subscribe(u_conn,"control");
	debug_print("subscribe\n");
	
	timer_set( &m->timer_umqtt_kepalive, CLOCK_SECOND * (u_conn->kalive));	// -1 ?
	timer_set( &m->timer_umqtt_publish, CLOCK_SECOND * (u_conn->kalive)*2);
}


void mqtt_new_packet(struct umqtt_connection *u_conn) {
	unsigned int rsize;
	struct mqtt_connection *m;

	m = (struct umqtt_connection *)u_conn->private;
	rsize = umqtt_circ_pop(&u_conn->txbuff, (uint8_t*)buf, UBUFFS);
	
	debug_print("send:%d ",rsize);
	
	if ( Send( m->socket, buf, rsize) == W5100_OK ) {
		timer_restart( &m->timer_umqtt_kepalive );
		debug_print("Ok!\n");
		} else {
		debug_print("Fail\n");
		u_conn->state = UMQTT_STATE_FAILED;
		DisconnectSocket(m->socket);
		CloseSocket(m->socket);
	}
}



struct umqtt_connection u_conn = {
	.kalive = 30,
	.txbuff = {
		.start = u_txbuff,
		.length = sizeof(u_txbuff),
	},
	.rxbuff = {
		.start = u_rxbuff,
		.length = sizeof(u_rxbuff),
	},
	.message_callback = mqtt_message_callback,
	.connected_callback = mqtt_connected_callback,
	.new_packet_callback = mqtt_new_packet,
	.clientid = "LT2",
	.state = UMQTT_STATE_FAILED,
	.private = NULL,
};

struct mqtt_connection m_conn = {
	.serv_addr = {192,168,1,39},
	.tcp_port = 1883,
	.socket = 0,
	.connection = &u_conn,

};


void mqtt_exec_int(struct mqtt_connection *m_conn ) {
	char msg[10];
	unsigned int rsize;		// received bytes counter

	struct umqtt_connection *u_conn;
	
	u_conn = (struct umqtt_connection*)m_conn->connection;
	
	switch ( SocketState(m_conn->socket) ) {
		case SKT_CLOSED:
//		printf_P( PSTR("try to open\n") );
		debug_print("try to open\n");
		if (OpenSocket(m_conn->socket, W5100_SKT_MR_TCP, 49152 ) == m_conn->socket)		// if successful opening a socket...
		{
			_delay_ms(1);
			printf_P( PSTR("try to connect\n") );
			if ( Connect(m_conn->socket, m_conn->serv_addr, m_conn->tcp_port) == W5100_OK) {
				printf_P( PSTR("Connected\n"));
				} else {
				debug_print("Not connected\n");
				DisconnectSocket(m_conn->socket);
				CloseSocket(m_conn->socket);
			}
		}
		break;

		case  W5100_SKT_SR_ESTABLISHED:					// if socket connection is established...
			switch (u_conn->state) {
				case UMQTT_STATE_FAILED:
					umqtt_connect(u_conn);
					cnt = 0;
					break;
				case UMQTT_STATE_CONNECTED:
					if ( timer_tryrestart( &m_conn->timer_umqtt_publish ) ) {
						timer_restart( &m_conn->timer_umqtt_kepalive );
						sprintf(msg,"%05d", cnt++);
						debug_print("pub %s\n",msg);
						umqtt_publish(u_conn, "test", (uint8_t *)msg, strlen(msg));
					} else if ( timer_tryrestart( &m_conn->timer_umqtt_kepalive ) ){
						debug_print("ping %d\n",u_conn->nack_ping);
						umqtt_ping(u_conn);
					}
					break;
				case UMQTT_STATE_CONNECTING:
				case UMQTT_STATE_INIT:
					break;
			}	// switch u_conn.state

			if (!umqtt_circ_is_empty(&u_conn->txbuff)) {
				mqtt_new_packet( u_conn );
			}
			
			// need to receive?
			rsize = ReceivedSize(m_conn->socket);
			if (rsize>0) {
				debug_print("receive:%d ",rsize);
				if (Receive(m_conn->socket, buf, rsize) != W5100_OK) {
					u_conn->state = UMQTT_STATE_FAILED;
					DisconnectSocket(m_conn->socket);
					CloseSocket(m_conn->socket);
					break;
				}
				#ifdef MQTT_DEBUG
				buffer_print(buf, rsize);
				#endif
				umqtt_circ_push(&u_conn->rxbuff, buf, rsize);
				umqtt_process(u_conn);
			}	// to receive
			break;	// W5100_SKT_SR_ESTABLISHED
		case  W5100_SKT_SR_FIN_WAIT:
		case  W5100_SKT_SR_CLOSING:
		case  W5100_SKT_SR_TIME_WAIT:
		case  W5100_SKT_SR_CLOSE_WAIT:
		case  W5100_SKT_SR_LAST_ACK:
				DisconnectSocket(m_conn->socket);
				CloseSocket(m_conn->socket);
				u_conn->state = UMQTT_STATE_FAILED;
			break;
	}	// switch SocketState
}

void mqtt_exec() {
	if (m_conn.connection->private == NULL) m_conn.connection->private = &m_conn;
	mqtt_exec_int( &m_conn );
}
