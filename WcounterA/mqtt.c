/*
 * network.c
 *
 * Created: 11.11.2016 18:14:04
 *  Author: Artp
 */ 

#include <util/delay.h>

#include "../umqtt/umqtt.h"
#include "../netw5100/netw5100.h"


#define UBUFFS 150

uint8_t u_rxbuff[UBUFFS];
uint8_t u_txbuff[UBUFFS];

void message_callback(struct umqtt_connection *uc, char *topic, uint8_t *data, int len)
{
	printf("%s %s\n", topic, data);
}

void connected_callback(struct umqtt_connection *u_conn){

	umqtt_subscribe(u_conn,"control");
	printf("subscribe");
	
	timer_set(&(u_conn->(private->timer_umqtt_kepalive)), CLOCK_SECOND * (u_conn->kalive));	// -1 ?
	timer_set(&(u_conn->(private->timer_umqtt_publish)), CLOCK_SECOND * (u_conn->kalive)*2);
	
	m_conn->state = MQTT_STATE_CONNECTED;	//?????????????????????????????????????????????

}

#ifdef NET_DEBUG
void buffer_print( char* buf, unsigned int len) {
	unsigned int i;
	for(i=0; i<len; i++) printf("0x%02x ", buf[i]);
	printf("\n");
}
#endif


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
	.message_callback = message_callback,
	.connected_callback = connected_callback,
	.clientid = "LT1/WC2",
	.state = UMQTT_STATE_FAILED,
	.private = &m_conn,
};

struct mqtt_connection m_conn = {
	.serv_addr = {192,168,1,39},
	.tcp_port = 1883,
	.socket = 0,
	.private = &u_conn,

};

void mqtt_exec() {
	mqtt_exec_int( &m_conn );
}

static unsigned int cnt;

void mqtt_exec_int(struct mqtt_connection *m_conn, ) {
	struct umqtt_connection *u_conn;
	u_conn = (struct umqtt_connection*)m_conn->private;
	
	switch ( SocketState(m_conn->socket) ) {
		case SKT_CLOSED:
		if (OpenSocket(m_conn->socket, W5100_SKT_MR_TCP, 49152 ) == m_conn->socket)		// if successful opening a socket...
		{
			_delay_ms(1);
			if ( Connect(m_conn->socket, m_conn->serv_addr, m_conn->tcp_port) == W5100_OK) {
				printf("Connected\n");
				} else {
				printf("Not connected\n");
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
					if ( timer_tryrestart(&(m_conn->timer_umqtt_publish)) ) {
						timer_restart( &(m_conn->timer_umqtt_kepalive) );
						sprintf(msg,"%05d", cnt++);
						printf("pub %s\n",msg);
						umqtt_publish(u_conn, "test", msg, strlen(msg));
					} else if ( timer_expired(&(m_conn->timer_umqtt_kepalive)) ){
						printf("ping %d\n",u_conn->nack_ping);
						umqtt_ping(u_conn);
					}
					break;
				case UMQTT_STATE_CONNECTING:
				case UMQTT_STATE_INIT:
					break;
			}
			if (!umqtt_circ_is_empty(u_conn->txbuff)) {
				rsize = umqtt_circ_pop(u_conn->txbuff, buf, UBUFFS);
				
				printf("send:%d ",rsize);
				if ( Send( m_conn->socket, buf, rsize) == W5100_OK ) {
					timer_restart( &(m_conn->timer_umqtt_kepalive) );
					printf("Ok!\n");
				} else {
					printf("Fail\n");
					u_conn->state = UMQTT_STATE_FAILED;
					DisconnectSocket(m_conn->socket);
					CloseSocket(m_conn->socket);
				}
			}
			rsize = ReceivedSize(m_conn->socket);
			if (rsize>0) {
				printf("receive:%d ",rsize);
				if (Receive(m_conn->socket, buf, rsize) != W5100_OK) {
					u_conn->state = UMQTT_STATE_FAILED;
					DisconnectSocket(m_conn->socket);
					CloseSocket(m_conn->socket);
					break;
				}
				buffer_print(buf, rsize);
				umqtt_circ_push(u_conn->rxbuff, buf, rsize);
				umqtt_process(u_conn);
			}
			break;
			case  W5100_SKT_SR_FIN_WAIT:
			case  W5100_SKT_SR_CLOSING:
			case  W5100_SKT_SR_TIME_WAIT:
			case  W5100_SKT_SR_CLOSE_WAIT:
			case  W5100_SKT_SR_LAST_ACK:
				DisconnectSocket(m_conn->socket);
				CloseSocket(m_conn->socket);
				u_conn->state = UMQTT_STATE_FAILED;
			break;
		}
	}
}