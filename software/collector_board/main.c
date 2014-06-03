#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>

#include "main.h"
#include "usart.h"
#include "board_support.h"
#include "interpret.h"
#include "ethernet_twi.h"
#include "zac.h"
#include "config.h"

void printarray(uint8_t * arr, uint8_t len){
  uint8_t i;
  for (i=0;i<len;i++){
    printf(" %x ", arr[i]);
  }
  printf("\n\r");
}

void io_init(void){
	LED1_DDR |= (1<<LED1);
	LED2_DDR |= (1<<LED2);
	LED3_DDR |= (1<<LED3);
	LED4_DDR |= (1<<LED4);

	SW1_DDR &= ~(1<<SW1);

	LED1_PORT |= (1<< LED1);
	LED2_PORT |= (1<< LED2);
	LED3_PORT |= (1<< LED3);
	LED4_PORT |= (1<< LED4);
}

int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	io_init();
	uart_init();

	sei();

	zac_init();

	struct config cfg;
	config_read(&cfg);

	twi_init(cfg.twi_addr);



	printf("Controller started\n\r");
	while (1) {
		twi_handle();
	}
}
