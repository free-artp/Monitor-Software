/*
 * adc.c
 *
 * Created: 15.09.2016 19:48:49
 *  Author: Artp
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../clock/timer.h"

struct timer timer_adc_sine;
struct timer timer_adc_int;

unsigned int Level;
uint8_t fl_new_data;


ISR(ADC_vect){
	static unsigned int adc_min = 1024, adc_max = 0, adc_data;
	static unsigned int adc_Int = 0, cnt_Int = 0;
	
	// Must read low first
	adc_data = ADCL | (ADCH << 8);

	// поиск максимума и минимума синуса
	adc_min = (adc_min < adc_data)? adc_min : adc_data;
	adc_max = (adc_max > adc_data)? adc_max : adc_data;

	if ( timer_tryrestart(&timer_adc_sine) ) {		// если намерили весь синус, то 
		adc_data = adc_max - adc_min;
		adc_max = 0;
		adc_min = 1024;
		if ( timer_tryrestart(&timer_adc_int) ) {	// усреднение
			if (!fl_new_data) {
				Level = adc_Int / cnt_Int;
				fl_new_data = true;
			}
			adc_Int = 0;
			cnt_Int = 0;
		} else {
			adc_Int += adc_data;
			cnt_Int++;
		}
	}

	// Set ADSC in ADCSRA (0x7A) to start the ADC conversion
	ADCSRA |=0b01000000;
}

void adc_init() {
	// clear ADLAR in ADMUX (0x7C) to right-adjust the result
	// ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
	ADMUX &= 0b11011111;
	
	// Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
	// proper source (01) - AVCC with external capacitor at AREF pin
	ADMUX |= 0b01000000;
	
	// Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
	// input
	ADMUX &= 0b11110000;
	
	// Set MUX3..0 in ADMUX (0x7C) to read from AD8 (Internal temp)
	// Do not set above 15! You will overrun other parts of ADMUX. A full
	// list of possible inputs is available in Table 24-4 of the ATMega328
	// datasheet
	// ADMUX |= 8;
	// ADMUX |= B00001000; // Binary equivalent
	//ADMUX |= 8;
	ADMUX |= 5;
	
	// Set ADEN in ADCSRA (0x7A) to enable the ADC.
	// Note, this instruction takes 12 ADC clocks to execute
	ADCSRA |= 0b10000000;
	
	#ifdef ADC_FREE_RUNNING
	// Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
	ADCSRA |= 0b00100000;
	
	// Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running.
	// This means that as soon as an ADC has finished, the next will be
	// immediately started.
	ADCSRB &= 0b11111000;
	#endif
	
	// Set the Prescaler to 128 (16000KHz/128 = 125KHz)
	// Above 200KHz 10-bit results are not reliable.
	ADCSRA |= 0b00000111;
	
	// Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
	// Without this, the internal interrupt will not trigger.
	ADCSRA |= 0b00001000;
	
	// Enable global interrupts
	// AVR macro included in <avr/interrupts.h>, which the Arduino IDE
	// supplies by default.
	// in setup()
	// sei();

	// setup timer for 10ms, чтобы промерить пол периода синуса
	timer_set( &timer_adc_sine, CLOCK_SECOND/100 );

	// setup timer, для усреднения 10-ти последовательных замеров (100 ms)
	timer_set( &timer_adc_int, CLOCK_SECOND/100 * 10 );
	
	fl_new_data = false;
	
	// Set ADSC in ADCSRA (0x7A) to start the ADC conversion
	ADCSRA |=0b01000000;
}

