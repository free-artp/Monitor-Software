/*
 * adc.h
 *
 * Created: 15.09.2016 19:54:34
 *  Author: Artp
 */ 


#ifndef ADC_H_
#define ADC_H_

#include "../clock/timer.h"

void adc_init(void);

extern unsigned int Level;
extern uint8_t fl_new_data;



#endif /* ADC_H_ */