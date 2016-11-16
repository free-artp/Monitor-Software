/*
 * adc.h
 *
 * Created: 15.09.2016 19:54:34
 *  Author: Artp
 */ 


#ifndef ADC_H_
#define ADC_H_

#include "../clock/timer.h"

void setup_ADC(void);

extern struct timer_adc_sine;
extern struct timer_adc_int;

extern unsigned int DATA;



#endif /* ADC_H_ */