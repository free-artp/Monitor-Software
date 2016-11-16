/* 
 * This file handles time keeping
 *
 * Copyright (C) Josef Gajdusek <atx@atx.name>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * */

#include <avr/interrupt.h>

#include "clock.h"

static clock_time_t time;

/* Overflow interrupt */
ISR(TIMER1_COMPA_vect)
{
	time += 1;
}

void clock_init()
{
	time = 0;
	/* Enable compare A interrupt */
	TIMSK1 |= _BV(OCIE1A);

	/* f_cpu / 1024 and CTC mode */
//	TCCR1B |= _BV(CS12) | _BV(CS10) | _BV(WGM12);
//	OCR1A = 6104; /* Magical constant - 2Hz */

	/* f_cpu / 256 and CTC mode */
	TCCR1B |= _BV(CS12) | _BV(WGM12);
//	OCR1A = 6250; /* Magical constant - 10Hz - 100 ms */
	OCR1A = 125; /* Magical constant - 50Hz - 2 ms */
}

clock_time_t clock_time()
{
	return time;
}

void clock_set(clock_time_t t)
{
	time = t;
}
