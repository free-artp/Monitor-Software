/*
 * debug.h
 *
 * Created: 16.11.2016 18:41:07
 *  Author: Artp
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>
#include <avr/pgmspace.h>

#ifdef CONSOLE_DEBUG
	#define debug_print(fmt, ...) \
	do { if (CONSOLE_DEBUG) fprintf(stdout, "%s(%lu): " fmt, __FUNCTION__, clock_time(), ##__VA_ARGS__); } while (0)


	#define debug_print_P(msg) \
	do { if (CONSOLE_DEBUG) fprintf_P(stdout, PSTR("%s(%lu): %S"), __FUNCTION__, clock_time(), PSTR(msg)  ); } while (0)

#else
	#define debug_print(fmt, ...) asm("nop")
	#define debug_print_P(fmt, ...) asm("nop")

#endif // DEBUG



#endif /* DEBUG_H_ */