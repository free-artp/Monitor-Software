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

unsigned int DATA; 

void setup_ADC() {
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

	// setup timer for 20ms, чтобы промерить весь синус
	// если будет выпрямленное, то вдвое меньше
	timer_set( &timer_adc_sine, CLOCK_SECOND/50 );

	// setup timer, для усреднения 10-ти последовательных замеров (200 ms)
	timer_set( &timer_adc_int, CLOCK_SECOND/50 * 10 );
	
	// Set ADSC in ADCSRA (0x7A) to start the ADC conversion
	ADCSRA |=0b01000000;
}

ISR(ADC_vect){
	static unsigned int adc_min = 1024, adc_max = 0, adc_data;
	static unsigned int adc_Int = 1024, cnt_Int = 0;
	
	// Must read low first
	adc_data = ADCL | (ADCH << 8);

	// поиск максимума и минимума синуса
	adc_min = (adc_min < adc_data)? adc_min : adc_data;
	adc_max = (adc_max > adc_data)? adc_max : adc_data;

	if ( timer_tryrestart(&timer_adc_sine) ) {		// если намерили весь синус, то 
//		DATA = adc_max - adc_min;
		DATA = adc_max;
		adc_max = 0;
		adc_min = 1024;
		if ( timer_tryrestart(&timer_adc_int) ) {	// усреднение
			DATA = adc_Int / cnt_Int;
			adc_Int = 0;
			cnt_Int = 0;
		} else {
			adc_Int += DATA;
			cnt_Int++;
		}
	}

	// Set ADSC in ADCSRA (0x7A) to start the ADC conversion
	ADCSRA |=0b01000000;
}


// Далее код из ардуиновского скетча
/*
// Interrupt service routine for the ADC completion
ISR(ADC_vect){
	static int cnt_adc = 0;
	static int adc_min = 1024, adc_max = 0, adc_data;
	
	// Must read low first
	adc_data = ADCL | (ADCH << 8);
	
	adc_min = (adc_min < adc_data)? adc_min : adc_data;
	adc_max = (adc_max > adc_data)? adc_max : adc_data;

	cnt_adc = (cnt_adc >= MAXADCM) ? 0 : ++cnt_adc ;

	if (cnt_adc == 0) {
		DATA = adc_max - adc_min;
		adc_max = 0;
		adc_min = 1024;
		if (cntInt < MAXINT) {
			Level_SUM += DATA;
			cntInt++;
			} else {
			cntInt = 0;
			Level = Level_SUM / MAXINT;
			Level_SUM = 0;
		}
	}
	// Set ADSC in ADCSRA (0x7A) to start another ADC
	// Not needed because free-running mode is enabled.

	// ---------------------- wood counter
	if (state_wood) {
		if (Level <= Level0 and len_wood >= minLen1) {
			state_wood = false;
			cnt_wood++;
			flag_end++;

			to_update = true;
			len_wood = 0L;
			pwr_wood = 0L;
		}
		} else {
		if (Level >= Level1) {
			state_wood = true;
			flag_beg ++;
			to_update = true;
		}
	}
}

*/

// ============================================== Timer interrupts
//
// 1 ms (0.001 sec)
// 1 замер АЦП примерно 100 мкс. Для получения валидного значения Level делаем 200 замеров.
// Т.е. оно появляется раз в 20 ms
// мы пробежим здесь 20 раз при одном и том же значении Level (это без USEMAXINT)
//
/*
void master() {

	//===============================================
	#ifdef OLDALG
	if (state_wood) {
		if ( Level <= Level0 & ( len_wood >= minLen1 ) ) {  // end of wood
			state_wood = false;
			len_pause = 0L;
			cnt_wood++;
			new_end = true;
		}
		} else {
		if ( (Level >= Level1) & ( len_pause >= minLen0 ) ) {  // begin of wood
			state_wood = true;
			len_wood = 0L;
			pwr_wood = 0L;
			new_beg = true;
		}
	}
	#endif

	#ifdef OLDALG2
	static unsigned long cnt_0=0, cnt_1=0;
	//  if (fl_new_val) {
	fl_new_val = false;

	if (!state_wood) {
		cnt_1 = (Level >= Level1) ? ++cnt_1 : 0L;
	}
	
	if (state_wood) {
		cnt_0 = (Level <= Level0) ? ++cnt_0 : 0L;
	}
	
	if ( (!state_wood) & (cnt_1 > minLen1) ) {  // begin of wood
		len_wood = cnt_1;
		cnt_1 = 0L;
		
		state_wood = true;
		new_beg = true;
	}
	
	if ( (state_wood) & (cnt_0 > minLen0) ) {  // end of wood
		len_wood -= cnt_0;
		cnt_0 = 0L;
		//    len_pause = 0L;
		
		state_wood = false;
		cnt_wood++;
		new_end = true;
	}
	
	if (state_wood) len_wood++;
	//  }
	#endif

	#ifdef NEWALG
	if (state_wood) {
		if (Level<=Level0) {            // уровень был высокий (WOOD) и упал. Типа END? Посмотрим как долго он будет низкий...
			--interval;
			if (  interval < -minLen0 ) {  // уровень низкий достаточно долго. Значит действительно END.
				state_wood = false;
				//        len_pause = 0L;
				len_wood  = 0L;      // interval в этом случае отрицательный
				interval = 0L;
				new_end = true;
				cnt_wood++;
			}
			} else {
			interval = 0L;                // возможно уровень и падал, но когда он упадет снова надо будет считать заново.
		}
		len_wood++;
		pwr_wood += Level;
		} else {
		if (Level>=Level1) {
			++interval;
			if ( interval > minLen1 ) {
				state_wood = true;
				len_wood = 0L;
				//        len_pause = 0L;    // inteval в этом случае положительный
				interval = 0L;
				pwr_wood = 0L;
				new_beg = true;
			}
			} else {
			interval = 0L;
		}
		len_pause++;
	}
	#endif


	#ifdef NEWALG2
	static unsigned long cnt_h=0, cnt_l=0;
	static bool state_h=false, state_l=false;

	if ( Level >= Level1 ) {
		cnt_h++;
		state_h = cnt_h >= minLen1;
		if (state_h) {
			cnt_h += cnt_l;
			cnt_l = 0L;
		}
	}
	if ( Level <= Level0 ) {
		cnt_l++;
		state_l = cnt_l >= minLen0;
		if (state_l) {
			cnt_l += cnt_h;
			cnt_h = 0L;
		}
	}

	if (!state_wood & state_h) {
		len_wood = cnt_h;
		len_pause -= cnt_h;
		state_wood = true;
		new_beg = true;
	}
	if (state_wood & state_l) {
		len_wood -= cnt_l;
		len_pause = cnt_l;
		cnt_wood++;
		state_wood = false;
		new_end = true;
	}
	
	if (state_wood) len_wood++ else len_pause++;
	
	#endif
	//------------------------  Engine
	if (state_engine){
		if (Level < LevelE0){
			state_engine = false;
			new_state_engine = true;
		}
		} else {
		if (Level >= LevelE1){
			state_engine = true;
			new_state_engine = true;
		}
	}

}

*/