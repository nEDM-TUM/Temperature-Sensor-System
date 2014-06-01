#include "ethernet_twi.c"


void twi_init(){
  // set slave address
  // also listen to general call
  TWAR = (SLA << 1) | 1;
  TWCR = (1<<TWEA) | (1<<TWEN);
}

void handle_communications(){
	// XXX: one has to be EXTREMELY careful when debugging this code with printf,
	// XXX: as the caused delay for printing influences the bus heavily.
  if(TWCR & (1<<TWINT)){
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
						do_measurement();
						cstate = IDLE;
						break;
					default:
						cstate = IDLE;
						// Switched to the not addressed Slave mode; own SLA will be recognized;
						TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
						break;
				}

				break;

	
			// slave transmitter:
			case 0xa8:
				// Own SLA+R has been received;
				// ACK has been returned
				switch (cstate){
					case IDLE:
						
						// this is the first byte of the transmission
						bufferpointer = 1;
						TWDR = ((uint8_t*)measurement_data )[0];
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
				TWDR = ((uint8_t*)measurement_data)[bufferpointer]; //data :)
				bufferpointer++;
				// Data byte will be transmitted and ACK should be received
				TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);

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
  }
}
