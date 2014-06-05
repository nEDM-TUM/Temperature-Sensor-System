#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include "configuration.h"
#include "collector_twi.h"

#include "packet.h"


#define SLA1 110
#define SLA2 111

uint8_t scanresults[20];
uint8_t num_boards;



void loop(){
	uint8_t s;
	PORTD ^= (1<<PD5);

	struct dummy_packet received[8];

	uint8_t state;
	int16_t temp;

	state = twi_start_measurement(0x00);
	uint8_t iaddr;
	uint8_t addr;
	for (iaddr=0;iaddr<num_boards;iaddr++){
		addr = scanresults[iaddr];
		printf("# %u # ", addr);
		//state = start_measurement(SLA2);
		state = twi_receive_data(addr, ((uint8_t*)received),8*sizeof(struct dummy_packet));
		if (state){
			for (s=0;s<8;s++){
				printf(" | P%u: ", s+1);
				if(received[s].header.error && received[s].header.connected){
					printf("ERROR: ");
				}
				if(received[s].header.connected){
					switch(received[s].header.type){
						case PACKET_TYPE_TSIC:
							temp =  ((struct tsic_packet *)(received) )[s].temperature;
							printf("T = %d.%02d", temp/100, temp%100);
							//printf(" T = %d", ( (struct tsic_packet *)(received) )[s].temperature);
							break;
						case PACKET_TYPE_HYT:
							printf("T = %d", ( (struct hyt_packet *)(received) )[s].temperature);
							printf(" H = %d", ( (struct hyt_packet *)(received) )[s].humidity);
							break;
						default:
							printf("unknown packet type");
							break;
					}
				}else{
					printf("---nc---");
				}
			}
		}
		printf("\n\r");
	}

	num_boards = twi_scan(scanresults, 20);
	// -- printf("0x%x: ", SLA2);
	// -- if (state){
	// -- 	//printarray((uint8_t*)received, 40);
	// -- 	for(s = 0; s<8; s++){
	// -- 		printf("P%u: ", s+1);
	// -- 		twi_interpret(received[s]);
	// -- 		printf(" | ");
	// -- 	}
	// -- 	printf("\n\r");
	// -- }else{
	// -- 	printf("receive interrupted\n\r");
	// -- }
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
	scanresults[20];
	num_boards = twi_scan(scanresults, 20);

	uint8_t i;
	printf("found boards: ");
	for (i=0;i<num_boards;i++){
		printf(" %u", scanresults[i]);
	}
	printf("\n\r");

	while (1) {
		loop();
	}
}
