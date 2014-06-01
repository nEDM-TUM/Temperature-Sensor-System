#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include "configuration.h"
#include "collector_twi.h"


#define SLA1 0x77
#define SLA2 0x78

void loop(){
	uint8_t s;
	PORTD ^= (1<<PD5);

	uint8_t received[8][5];

	uint8_t state;

	printf("----------\n\r");
	state = twi_start_measurement(0x00);
	//state = start_measurement(SLA2);
	state = twi_receive_data(SLA2, ((uint8_t*)received),40);
	printf("0x%x: ", SLA2);
	if (state){
		//printarray((uint8_t*)received, 40);
		for(s = 0; s<8; s++){
			printf("P%u: ", s+1);
			twi_interpret(received[s]);
			printf(" | ");
		}
		printf("\n\r");
	}else{
		printf("receive interrupted\n\r");
	}
	// -- state = receive_data(SLA2, ((uint8_t*)received),40);
	// -- printf("\n\r");
	// -- printf("0x%x: ", SLA2);
	// -- if (state){
	// -- 	//printarray((uint8_t*)received, 40);
	// -- 	for(s = 0; s<8; s++){
	// -- 		printf("P%u: ", s+1);
	// -- 		interpret(received[s]);
	// -- 		printf(" | ");
	// -- 	}
	// -- 	printf("\n\r");
	// -- }else{
	// -- 	printf("receive interrupted\n\r");
	// -- }
}


int main (void)
{

  init();
	uart_init();
	sei();

	DDRD = (1<<PD5);

	printf("Controller started\n\r");
  //setupServerLib();
  setupServer();
	while (1) {
	//	loop();
	}
}
