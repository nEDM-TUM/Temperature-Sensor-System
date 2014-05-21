#include <avr/io.h>
#include <util/delay.h>
#include "usart.h"
#include "stdlib.h"


int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;


	uart_init();

	DDRC = (1<<PC0);
	DDRC |= (1<<PC1);
	PORTC |= (1<<PC0);

	printf("Controller started\n\r");
	while (1) {
		PORTC ^= (1<<PC0);
		PORTC ^= (1<<PC1);
		_delay_ms(500);
	}
}
