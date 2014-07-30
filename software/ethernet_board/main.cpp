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
#include "config.h"
#include <avr/wdt.h>

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
			if(get_time_delta(time_last_measurement, current_time) >= cfg.measure_interval){
        // time is ready, we have to do measurement
        //puts_P(PSTR("t=0\n\r"));
				if(twi_try_lock_bus()){
          // ##########
          // BUS LOCKED
          // ##########
          //puts_P(PSTR("lck,"));
					time_last_measurement = current_time;
					num_boards = twi_scan(scanresults, 20);
					twi_start_measurement(0x00);
					loop_current_board = 0;
					if(loop_current_board < num_boards){
            // we have boards :)
            //puts_P(PSTR(",brd"));
						time_receive_start = current_time;
						addr_current_board = scanresults[loop_current_board];
						rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), TWI_RCV_START);
						if(rcv_state != TWI_RCV_ERROR){
              //puts_P(PSTR("ok\n\r"));
							loop_state = LOOP_MEASURE;
              // bus is still locked!!
						}else{
              puts_P(PSTR("ERR\n\r"));
							twi_free_bus();
              // ////////
              // BUS FREE
              // ////////
						}
					}else{
            puts_P(PSTR("NOlck,"));
						twi_free_bus();
            // ////////
            // BUS FREE
            // ////////
					}
				}else{
            puts_P(PSTR("lock error\n\r"));
        }
			}
			break;

		case LOOP_MEASURE:

			rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), rcv_state);
			switch (rcv_state){
				case TWI_RCV_FIN:
          //puts_P(PSTR("f"));
          // DBG here
#ifdef DEBUG
					puts_P(PSTR("measurement finished\n\r"));
#endif
          // --------------------------------------------
          // we have received data from a collector board
          // --------------------------------------------
          // verify checksums:
					twi_verify_checksums(received, 8);
          //puts_P(PSTR("c"));
          // hand packets to network layer:
          // FIXME: this is blocking after 1 to 8 hous and not releasing!!
          // watchdog should do a system reset if this happens, but this
          // is just a workaround.
					net_dataAvailable(received, addr_current_board);
          //puts_P(PSTR("d"));

					// switch to next board:
					loop_current_board ++;
          //if(num_boards < 3){ // XXX just for debug
          //  puts_P(PSTR("not brd\n\r"));
          //}
					if(loop_current_board < num_boards){
						addr_current_board = scanresults[loop_current_board];
						rcv_state = twi_try_receive_data(addr_current_board, ((uint8_t*)received),8*sizeof(struct dummy_packet), TWI_RCV_START);
						if(rcv_state != TWI_RCV_ERROR){
							loop_state = LOOP_MEASURE;
						}else{
              puts_P(PSTR("x"));
							// FIXME: this wil abort every succeeding board readout if one fails
              // FIXME: do we need to set rcv_state here to valid value?
              // should not be a problem, because we go to loop_idle ...
							twi_free_bus();
              // ////////
              // BUS FREE
              // ////////
							loop_state = LOOP_IDLE;
						}
					}else{
						twi_free_bus();
            // ////////
            // BUS FREE
            // ////////
						loop_state = LOOP_IDLE;
					}
          //puts_P(PSTR("s\n\r"));
					break;
				case TWI_RCV_ERROR:
          puts_P(PSTR("rcERR\n\r"));
					// receive encountered an error
					// FIXME: this wil abort every succeeding board readout if one fails
					twi_free_bus();
          // ////////
          // BUS FREE
          // ////////
					loop_state = LOOP_IDLE;
					break;
				default:
          //puts_P(PSTR("rcv\n\r"));
					// we still have to receive:
					loop_state = LOOP_MEASURE;
					if(get_time_delta(time_receive_start, current_time) >= 800){
						// timeout occured, the collector board takes too long to return data!
						puts_P(PSTR("loop timeout\n\r"));
						// we have to abort everything as we might already have violated time
						twi_free_bus();
            // ////////
            // BUS FREE
            // ////////
						loop_state = LOOP_IDLE;
					}
					break;
			}

			break;
	}
}


int main (void)
{

  wdt_enable(WDTO_2S);
  init();
	uart_init();
	// fast SPI mode:
	SPI.setClockDivider(4);
	sei();



	DDRD = (1<<PD5);
  DDRB = (1<<PB1);


  PORTB &= ~(1<<PB1);
  PORTD &= ~(1<<PD5);
  //_delay_ms(1000); //debug only XXX


	printf_P(PSTR("Controller started\n\r"));
	num_boards = twi_scan(scanresults, 20);

	uint8_t i;
	printf_P(PSTR("found boards: "));
	for (i=0;i<num_boards;i++){
		printf_P(PSTR(" %u"), scanresults[i]);
	}
	puts_P(PSTR("\n\r"));

  net_setupServer();

  wdt_reset();
  uint16_t dcnt = 0;
	// main event loop
	while (1) {
    //dcnt ++;
    //if(dcnt >= 1000){
    //  PORTB ^= (1<<PB1);
    //  dcnt = 0;
    //}
		loop_sampling();
    wdt_reset();
		net_loop();
    wdt_reset();
	}
}
