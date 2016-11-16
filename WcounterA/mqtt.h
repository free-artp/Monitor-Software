/*
 * mqtt.h
 *
 * Created: 11.11.2016 18:54:01
 *  Author: Artp
 */ 


#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_TEST_MESAGES 1

#include "../clock/timer.h"
#include "../umqtt/umqtt.h"


struct mqtt_connection {
	struct umqtt_connection *connection;
	
	unsigned char serv_addr[4];
	unsigned int tcp_port;
	unsigned char socket;

	struct timer timer_umqtt_kepalive;
#ifdef MQTT_TEST_MESAGES
	struct timer timer_umqtt_publish;
#endif
};

void mqtt_exec();
void mqtt_publish(char *topic, uint8_t *data, int datalen);

char * mqtt_full_topic_P(char * topic);

#endif /* MQTT_H_ */