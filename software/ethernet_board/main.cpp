// this is for macros like UINT32_MAX to be available
#define __STDC_LIMIT_MACROS

#include <avr/io.h>
#include <Arduino.h>
#include <util/delay.h>
#include "usart.h"
#include "networking.h"
#include "collector_twi.h"
#include "packet.h"
#include "SPI.h"

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

void loop_sampling(){
	uint32_t current_time = millis();
	switch (loop_state){
		case LOOP_IDLE:
			if(get_time_delta(time_last_measurement, current_time) >= measure_interval){
#ifdef DEBUG
				puts(PSTR("start!\n\r"));
#endif
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
#ifdef DEBUG
					puts_P(PSTR("measurement finished\n\r"));
#endif
					twi_verify_checksums(received, 8);

          // TODO: implement CRC check here
					net_dataAvailable(received, addr_current_board);
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
						puts_P(PSTR("loop timeout\n\r"));
						// we have to abort everything as we might already have violated time
						twi_free_bus();
						loop_state = LOOP_IDLE;
					}
					break;
			}

			break;
	}
}


int main (void)
{

  init();
	uart_init();
	// fast SPI mode:
	SPI.setClockDivider(4);
	sei();

	DDRD = (1<<PD5);

	puts_P(PSTR("Controller started\n\r"));
	num_boards = twi_scan(scanresults, 20);

	uint8_t i;
	puts_P(PSTR("found boards: "));
	for (i=0;i<num_boards;i++){
		printf_P(PSTR(" %u"), scanresults[i]);
	}
	puts_P(PSTR("\n\r"));

  net_setupServer();

	// main event loop
	while (1) {
		loop_sampling();
		net_loop();
	}
}
