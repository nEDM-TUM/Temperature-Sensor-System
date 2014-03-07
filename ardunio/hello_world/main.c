#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include <inttypes.h>


int main (void)
{

	DDRB = 0xff;
	PORTB = 0;
	uint8_t t = 1;
	while (1) {
		PORTB = PORTB ^ (1<<PB1);
		_delay_ms(100);

	}
}
