#include "ethernet_twi.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "zac.h"
#include "interpret.h"
#include <avr/io.h>
#include "config.h"
#include "board_support.h"

uint8_t cstate = IDLE;

void twi_init(uint8_t addr){
	// reset TWI hardware:
	TWCR = 0;
	_delay_ms(1);
	end_of_transmit = interpreted_data;
  // set slave address
  // also listen to general call
  TWAR = (addr << 1) | 1;
	// start responding:
  TWCR = (1<<TWEA) | (1<<TWEN);
}

void twi_handle(){
	// XXX: one has to be EXTREMELY careful when debugging this code with printf,
	// XXX: as the caused delay for printing influences the bus heavily.
  if(TWCR & (1<<TWINT)){
    LED2_PORT ^= (1<<LED2);
    //TWI interrupt
    switch (TWSR){
			// slave receiver:
			case 0x70:
				// General call address has been
				// received; ACK has been returned
			case 0x60:
				// Own SLA+W has been received;
				// ACK has been returned
				switch (cstate){
          // FIXME: do not do a case switch here,
          // if we are addressed, always go to command mode!
					case IDLE:
						// Data byte will be received and ACK will be returned
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
						cstate = COMMAND;
						break;
					default:
						// Data byte will be received and NOT ACK will be returned
						TWCR = (1<<TWEN) | (1<<TWINT);
				}
				break;
			case 0x68:
				// Arbitration lost in SLA+R/W as
				// Master; own SLA+W has been
				// received; ACK has been returned
			case 0x78:
				// Arbitration lost in SLA+R/W as
				// Master; General call address has
				// been received; ACK has been
				// returned

				// Data byte will be received and NOT ACK will be returned
				TWCR = (1<<TWEN) | (1<<TWINT);
				printf("ERROR 0x%x\n\r", TWSR);
				break;

			case 0x90:
				// Previously addressed with
				// general call; data has been re-
				// ceived; ACK has been returned
			case 0x80:
				// Previously addressed with own
				// SLA+W; data has been received;
				// ACK has been returned
				switch (cstate){
					case COMMAND:
						switch (TWDR){
							case CMD_START_MEASUREMENT:
								// Data byte will be received and NOT ACK will be returned
								TWCR = (1<<TWEN) | (1<<TWINT);
								cstate = START_MEASUREMENT;
								break;
							case CMD_SET_ADDRESS:
								// Data byte will be received and ACK will be returned
								TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
								cstate = WAIT_ADDRESS;
								break;
							default:
								// Data byte will be received and ACK will be returned
								TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
								break;
						}
						break;

					case WAIT_ADDRESS:
						// Data byte will be received and NOT ACK will be returned
            cstate = ADDR_FIN;
            cfg.twi_addr = TWDR;
						TWCR = (1<<TWEN) | (1<<TWINT);

					default:
						// Data byte will be received and NOT ACK will be returned
						TWCR = (1<<TWEN) | (1<<TWINT);
				}
				break;

			case 0x98:
				// Previously addressed with
				// general call; data has been
				// received; NOT ACK has been
				// returned
			case 0x88:
				// Previously addressed with own
				// SLA+W; data has been received;
				// NOT ACK has been returned
				cstate = IDLE;
				// Switched to the not addressed Slave mode; own SLA will be recognized;
				TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);

				break;

			case 0xa0:
				// A STOP condition or repeated
				// START condition has been
				// received while still addressed as
				// Slave
				switch (cstate){
					case START_MEASUREMENT:
						// Switched to the not addressed Slave mode; own SLA will be recognized;
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
						// Start meassuring process. this will block
						// no new twi activity will be processed.
						// If new command arrives, clock will
						// be extended, until measurement is completed
						LED4_PORT &= ~(1<<LED4);
						uint8_t connected = zac_sampleAll(measurement_data);
						end_of_transmit = interpret_generatePacketAll(measurement_data, connected, interpreted_data);
						LED4_PORT |= (1<<LED4);
						//interpret_detectPrintAll(measurement_data, connected);
						cstate = IDLE;
						break;
          case ADDR_FIN:
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
            LED1_PORT ^= (1<<LED1);
            config_write(&cfg);
            twi_init(cfg.twi_addr);
            cstate = IDLE;
					default:
						cstate = IDLE;
						// Switched to the not addressed Slave mode; own SLA will be recognized;
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
						break;
				}
				printf("stop %u\n\r", cstate);

				break;

	
			// slave transmitter:
			case 0xa8:
				// Own SLA+R has been received;
				// ACK has been returned
				switch (cstate){
					case IDLE:
						
						// this is the first byte of the transmission
						bufferpointer = 1;
						TWDR = interpreted_data[0];
						// Data byte will be transmitted and ACK should be received
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
						// FIXME: TRANSMIT state is never used
						cstate = TRANSMIT;

						break;
					default:
						TWDR = 0xff;
						// Last data byte will be transmitted and NOT ACK should be received
						TWCR = (1<<TWEN) | (1<<TWINT);

						break;
				}

				break;
			case 0xb0:
				// Arbitration lost in SLA+R/W as
				// Master; own SLA+R has been
				// received; ACK has been returned

				printf("ERROR 0xb0\n\r");
				break;
			case 0xb8:
				// Data byte in TWDR has been
				// transmitted; ACK has been
				// received
				TWDR = interpreted_data[bufferpointer]; //data :)
				bufferpointer++;
				if(interpreted_data + bufferpointer < end_of_transmit){
					// Data byte will be transmitted and ACK should be received
					TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
				}else{
					// Last Data byte will be transmitted and NOT ACK should be received
					TWCR = (1<<TWEN) | (1<<TWINT);
				}

				break;
			case 0xc0:
				// Data byte in TWDR has been
				// transmitted; NOT ACK has been
				// received

				// Switched to the not addressed Slave mode; own SLA will be recognized;
				TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
				cstate = IDLE;
				break;
			case 0xc8:
				// Last data byte in TWDR has been
				// transmitted (TWEA = “0”); ACK
				// has been received

				// Switched to the not addressed Slave mode; own SLA will be recognized;
				TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
				cstate = IDLE;
				break;



			case 0x00:
				// Bus error due to an illegal
				// START or STOP condition

				// this might happen, if device is unplugged and then
				// plugged back into the bus
				//printf("bus error, state is %d\n\r", cstate);
				cstate = IDLE;

				// Only the internal hardware is affected, no STOP condi-
				// tion is sent on the bus. In all cases, the bus is released
				// and TWSTO is cleared.
        TWCR = (1<< TWEA) | (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
				break;
			case 0xf8:
				printf("no state change detected\n\r");
				break;
			default:
				printf("ERROR: unknown TWI state\n\r");
				break;

      
    }
		printf("cstate %u\n\r", cstate);
  }
}
