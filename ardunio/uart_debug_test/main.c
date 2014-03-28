#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include "strfun.h"
#include "usart.h"




int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;


	uart_init();

	printf("Controller started\n\r");
	while (1) {
		_delay_ms(500);
		printf("here\n\r");
	}
}
