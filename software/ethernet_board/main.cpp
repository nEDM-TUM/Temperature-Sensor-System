#define __STDC_LIMIT_MACROS
#include <avr/io.h>
#include <Arduino.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include "configuration.h"
#include "collector_twi.h"
#include "packet.h"


#include <socket.h>

#define SLA1 110
#define SLA2 111

#define LOOP_MEASURE 1
#define LOOP_IDLE 2


uint8_t scanresults[20];
uint8_t num_boards;


uint8_t loop_state = LOOP_IDLE;
uint8_t loop_current_board;
uint8_t addr_current_board;
uint8_t rcv_state;
uint32_t time_last_measurement = 0;
uint32_t time_receive_start;

struct dummy_packet received[8];



uint32_t get_time_delta(uint32_t a, uint32_t b){
  if(a>b){
    return  UINT32_MAX - a + b;
  }else{
    return b-a;
  }
}

void loop2(){
	uint32_t current_time = millis();
	switch (loop_state){
		case LOOP_IDLE:
			if(get_time_delta(time_last_measurement, current_time) >= measure_interval){
				printf("start!\n\r");
				if(twi_try_lock_bus()){
					time_last_measurement = current_time;
					num_boards = twi_scan(scanresults, 20);
					twi_start_measurement(0x00);
					loop_current_board = 0;
					if(loop_current_board < num_boards){
						time_receive_start = current_time;
						addr_current_board = scanresults[loop_current_board];
						rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), TWI_RCV_START);
						if(rcv_state != TWI_RCV_ERROR){
							loop_state = LOOP_MEASURE;
						}else{
							twi_free_bus();
						}
					}else{
						twi_free_bus();
					}
				}
			}
			break;

		case LOOP_MEASURE:

			rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), rcv_state);
			switch (rcv_state){
				case TWI_RCV_FIN:
					printf("measurement finished\n\r");
					dataAvailable(received, addr_current_board);
					// switch to next board:
					loop_current_board ++;
					if(loop_current_board < num_boards){
						addr_current_board = scanresults[loop_current_board];
						rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), TWI_RCV_START);
						if(rcv_state != TWI_RCV_ERROR){
							loop_state = LOOP_MEASURE;
						}else{
							// FIXME: this wil abort every succeeding board readout if one fails
							twi_free_bus();
							loop_state = LOOP_IDLE;
						}
					}else{
						twi_free_bus();
						loop_state = LOOP_IDLE;
					}
					break;
				case TWI_RCV_ERROR:
					// receive encountered an error
					// FIXME: this wil abort every succeeding board readout if one fails
					twi_free_bus();
					loop_state = LOOP_IDLE;
					break;
				default:
					// we still have to receive:
					loop_state = LOOP_MEASURE;
					if(get_time_delta(time_receive_start, current_time) >= 800){
						// timeout occured, the collector board takes too long to return data!
						printf("loop timeout\n\r");
						// we have to abort everything as we might already have violated time
						twi_free_bus();
						loop_state = LOOP_IDLE;
						// FIXME: what will be the TWI state here?
					}
					break;
			}

			break;
	}
}


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
	num_boards = twi_scan(scanresults, 20);

	uint8_t i;
	printf("found boards: ");
	for (i=0;i<num_boards;i++){
		printf(" %u", scanresults[i]);
	}
	printf("\n\r");

  setupServer();

	// main event loop
	while (1) {
		loop2();
		ui_loop();
	}
}
