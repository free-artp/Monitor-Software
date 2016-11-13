/*
 * mqtt.h
 *
 * Created: 11.11.2016 18:54:01
 *  Author: Artp
 */ 


#ifndef MQTT_H_
#define MQTT_H_

#include "../clock/timer.h"


struct mqtt_connection {
	void *private;
	
	unsigned char serv_addr[4];
	unsigned int tcp_port;
	unsigned char socket;

	struct timer timer_umqtt_kepalive;
	struct timer timer_umqtt_publish;
};

void mqtt_exec();


#endif /* MQTT_H_ */