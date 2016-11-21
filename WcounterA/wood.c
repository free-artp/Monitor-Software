/*
 * wood.c
 *
 * Created: 21.11.2016 19:42:32
 *  Author: Artp
 */

#include <avr/pgmspace.h>

#include <string.h>
#include <stdio.h>

#include "mqtt.h"
#include "adc.h"
#include "../clock/timer.h"

static uint8_t operate = false;
static uint8_t state_wood, new_beg, new_end;
static unsigned int cnt_wood;
static unsigned long len_wood, len_pause;

static unsigned int Level0 = 100, Level1 = 500;
static unsigned int minLen0 = 10, minLen1 = 10;		// см.описание wood_exec. —ейчас 10 == 1сек.
static unsigned long cnt_0 = 0L, cnt_1 = 0L;

static struct timer timer_adc;

char msg[20];


void wood_init(){
	timer_set( &timer_adc, CLOCK_SECOND * 1 );
}

//
// частота по€влени€ fl_new_data зависит от timer_adc_sine (10ms) и timer_adc_int (10ms)
// т.е. раз в 100 ms.
//
void wood_exec() {

	if (fl_new_data) {

		if (!state_wood) {
			cnt_1 = (Level >= Level1) ? ++cnt_1 : 0;
		}
		
		if (state_wood) {
			cnt_0 = (Level <= Level0) ? ++cnt_0 : 0;
		}
		
		if (state_wood) {
			if (cnt_0 > minLen0)  {  // end of wood
				len_pause = cnt_0;
				len_wood -= cnt_0;
				cnt_0 = 0L;
				state_wood = false;
				new_end = true;
				cnt_wood++;
			} else {
				len_wood++;
			}
		} else {
			if (cnt_1 > minLen1)  {  // begin of wood
				len_wood = cnt_1;
				len_pause -= cnt_1;
				cnt_1 = 0L;
				state_wood = true;
				new_beg = true;
			} else {
				len_pause++;
			}
		}
		
		fl_new_data = false;
		
	}	// fl_new_data
}
	
void wood_msg() {
	if ( timer_tryreset(&timer_adc) ) {
		sprintf( msg, "%u", Level);
		mqtt_publish( mqtt_full_topic_P(PSTR("Level")), (uint8_t *)msg, strlen(msg));
	}
	if (new_beg) {
		sprintf( msg, "%u %lu", cnt_wood+1, len_pause);		// cnt, len_pause
		new_beg = false;
		mqtt_publish( mqtt_full_topic_P(PSTR("cnt/beg")), (uint8_t *)msg, strlen(msg));
	}
	if (new_end) {
		sprintf( msg, "%u 0 %lu", cnt_wood, len_wood);		// cnt, pwr, len_wood
		new_end = false;
		mqtt_publish( mqtt_full_topic_P(PSTR("cnt/end")), (uint8_t *)msg, strlen(msg));
	}
}

// ƒалее код из ардуиновского скетча
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
// 1 замер ј÷ѕ примерно 100 мкс. ƒл€ получени€ валидного значени€ Level делаем 200 замеров.
// “.е. оно по€вл€етс€ раз в 20 ms
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
		if (Level<=Level0) {            // уровень был высокий (WOOD) и упал. “ипа END? ѕосмотрим как долго он будет низкий...
			--interval;
			if (  interval < -minLen0 ) {  // уровень низкий достаточно долго. «начит действительно END.
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