#include "collector_twi.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include "../util/checksum.h"

uint8_t scanresults[20];
uint8_t num_boards;

// this is not a interrupt/threadsafe mutex
// this only works for event loop code
uint8_t twi_bus_mutex;

uint8_t twi_try_lock_bus(){
	if (twi_bus_mutex == 0){
		twi_bus_mutex = 1;
		//printf_P(PSTR(" bus locked\n\r"));
		return 1;
	}else{
		//printf_P(PSTR(" bus busy\n\r"));
		return 0;
	}
}

void twi_free_bus(){
	twi_bus_mutex = 0;
}

void printarray(uint8_t * arr, uint8_t len){
  uint8_t i;
  for (i=0;i<len;i++){
    printf(" %x ", arr[len-1-i]);
  }
  printf("\n\r");
}

uint8_t twi_computeCRC(uint8_t * data, uint8_t len, uint8_t crc){
  uint8_t result = data[0];
  uint8_t byte;
  int8_t i=1;
  int8_t index;
  while (1){
    if(i<len){
      byte = data[i];
    }else if(i == len){
      byte = crc;
    }else{
      break;
    }
    for(index=7; index >= 0; index--){
      if(result & (1<<7)){
        result = result << 1;
        result |= ((byte >> index) & 1);
        result ^= CRC8;
      }else{
        result = result << 1;
        result |= ((byte >> index) & 1);
      }
    }
    i++;
  }
  //printf_P(PSTR("\t\t\t!!!Result is %x\n\r"), result);
  return result;
}

uint8_t twi_wait(void){
	while(!(TWCR & (1<<TWINT))){
		//printf_P(PSTR("wait: TWSR = %x\n\r"), TWSR);
		// wait for interrupt
	}
	return 1;
}

uint8_t twi_wait_timeout(uint16_t milliseconds){
	uint16_t i, ms;
	i = 0;
  ms = 0;
	while(!(TWCR & (1<<TWINT))){
    _delay_us(5);
    if(i>=2000){
      i=0;
      ms++;
      if (ms>milliseconds){
        return 0;
      }
    }
    i++;
		// wait for interrupt
	}
	return 1;
}

uint8_t twi_scan(uint8_t * result, uint8_t max_results){
	uint8_t i;
	uint8_t n = 0;
	// scan all allowed addresses
	for (i=8;i<120;i++){
		if(twi_start()){
			
			TWDR = (i<<1);
			TWCR = (1<<TWINT) | (1<<TWEN);
			if(!twi_wait_timeout(2)){
				// if timeout happens, someone is blocking the bus
				// we immediately give up.
				return 0;
			}
			if(TWSR == 0x18){
				result[n] = i;
				n++;
				if (n >= max_results){
					// send stop
					TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
					return n;
				}
			}
			// send stop
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		}
	}
	return n;
}

uint8_t twi_start(){
	//do{
		//TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		//_delay_us(200);
    _delay_us(10);
		TWBR = 20;
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
		twi_wait_timeout(5);
	//}while(TWSR!=0x08);
	if (TWSR != 0x08){
		TWCR = 0;
		return 0;
	}else{
		return 1;
	}
	//return twi_wait_timeout(5);
	//while(!(TWCR & (1<<TWINT)) || (TWSR != 0x08) ){
	//	// wait for interrupt
	//	// and right status code
	//}
}

uint8_t twi_send_cmd(uint8_t addr, uint8_t cmd){
	// send start condition:
	if (!twi_start()){
		TWCR = (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
		printf_P(PSTR("could not send start\n\r"));
		return 0;
	}
	// send SLA + W
	TWDR = (addr<<1);
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
	switch (TWSR){
		case 0x18:
			// SLA+W has been transmitted;
			// ACK has been received
			TWDR = cmd;
			TWCR = (1<<TWINT)|(1<<TWEN);
			twi_wait_timeout(5);
			switch(TWSR){
				case 0x28:
					// Data byte has been transmitted;
					// ACK has been received
					return 1;
					break;
				case 0x30:
					// Data byte has been transmitted;
					// NOT ACK has been received
					TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
					printf_P(PSTR("error: 0x30\n\r"));
					return 0;
					break;
				default:
					//TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
					printf_P(PSTR("bus error2: tate = %x\n\r"), TWSR);
					return 0;
					break;
			}
			break;
		case 0x20:
			// SLA+W has been transmitted;
			// NOT ACK has been received
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
			printf_P(PSTR("error: 0x20\n\r"));
			return 0;
			break;
		case 0x38:
			// Arbitration lost in SLA+W or data bytes

			// 2-wire Serial Bus will be released and not addressed
			// Slave mode entered
			TWCR = (1<<TWINT) | (1<<TWEN);
			printf_P(PSTR("error: 0x38\n\r"));
			return 0;
			break;
		case 0xf8:
			// No relevant state information
			// available; TWINT = “0”
			// Timeout has occured, no slave is answering

			// FIXME: I do not know how to free the bus in case
			// of a timeout. START and SLA+W has already been sent
			// so I try to send a STOP condition here
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
			_delay_us(10);
			TWCR = 0;
			//printf_P(PSTR("error: 0xf8, no new state (timeout)\n\r"));
			return 0;
			break;
		default:
			TWCR = 0;
			printf_P(PSTR("bus error: state = %x\n\r"), TWSR);
			return 0;
			break;
	}
	
}

uint8_t twi_set_address(uint8_t addr, uint8_t new_addr){	
  if(!twi_send_cmd(addr, CMD_SET_ADDRESS)){
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		printf_P(PSTR("cmd not successful\n\r"));
    return 0;
  }
  TWDR = new_addr;
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
  if(TWSR == 0x28){
		// Data byte has been transmitted;
		// ACK has been received
		//_delay_ms(5);
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return 1;
  }else{
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		printf_P(PSTR("new addr not successful\n\r"));
    return 0;
  }
}

uint8_t twi_set_led(uint8_t addr, uint8_t on, uint8_t num){
	uint8_t cmd;
	if(on){
		cmd = CMD_LED_ON;
	}else{
		cmd = CMD_LED_OFF;
	}
  if(!twi_send_cmd(addr, cmd)){
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		printf_P(PSTR("cmd not successful\n\r"));
    return 0;
  }
  TWDR = num;
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
  if(TWSR == 0x28){
		// Data byte has been transmitted;
		// ACK has been received
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return 1;
  }else{
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		printf_P(PSTR("new addr not successful\n\r"));
    return 0;
  }

}

uint8_t twi_start_measurement(uint8_t addr){
  if(twi_send_cmd(addr, CMD_START_MEASUREMENT)){
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return 1;
  }else{
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return 0;
  }
}

uint8_t twi_verify_checksums(struct dummy_packet * packet, uint8_t num){
	uint8_t err = 0;
	while(num>0){
		if(checksum_computeCRC((uint8_t*)packet, 5, packet->crc) !=0){
			(packet->header).error = 2;
			err = 1;
		}
		packet++;
		num--;
	}
	return err;

}

uint8_t twi_try_receive_data(uint8_t address, uint8_t * buffer, uint8_t len, uint8_t state){
	switch(state){
		case TWI_RCV_START:
			if (!twi_start()){
				TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
				return TWI_RCV_ERROR;
			}
			TWDR = (address<<1) | 1;
			// send read request:
			TWCR = (1<<TWINT) | (1<<TWEN);
			twi_wait_timeout(5);
			switch(TWSR){
				case 0x38:
					// Arbitration lost in SLA+R or NOT ACK bit

					// 2-wire Serial Bus will be released and not addressed
					// Slave mode will be entered
					TWCR = (1<<TWINT)|(1<<TWEN);
					printf_P(PSTR("error 0x38\n\r"));
					return TWI_RCV_ERROR;
					break;
				case 0x40:
					// SLA+R has been transmitted;
					// ACK has been received

					// Data byte will be received and ACK will be returned
					TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
					return TWI_RCV_RECEIVE;
					break;
				case 0x48:
					// SLA+R has been transmitted;
					// NOT ACK has been received

					// STOP condition will be transmitted and TWSTO Flag
					// will be reset
					TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

					printf_P(PSTR("error 0x48\n\r"));
					return TWI_RCV_ERROR;
					break;
				case 0xf8:
				default:
					// No relevant state information
					// available; TWINT = “0”
					// FIXME: this reaction causes a bus error
					TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
					printf_P(PSTR("no new state\n\r"));
					return TWI_RCV_ERROR;
					break;
			}
			break;
		case TWI_RCV_RECEIVE:
			if (TWCR & (1<<TWINT)){
				while(TWSR == 0x50 && len > 0){
					//printf_P(PSTR("byte received!\n\r"));
					*buffer = TWDR;
					buffer++;
					len--;
					if (len == 1){
						TWCR = (1<<TWINT)|(1<<TWEN);
					}else{
						TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
					}
					if(!twi_wait_timeout(5)){
						printf_P(PSTR("timeout\n\r"));
						TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
						return TWI_RCV_ERROR;
					}
					//printf_P(PSTR("TWSR = %x\n\r"), TWSR);
				}
				*buffer = TWDR;
				TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
				//printf_P(PSTR("len = %d\n\r", len));
				if(len==1){
					return TWI_RCV_FIN;
				}else{
					return TWI_RCV_ERROR;
				}
			}else{
				return TWI_RCV_RECEIVE;
			}
	}
}

uint8_t twi_receive_data(uint8_t address, uint8_t * buffer, uint8_t len){
	//printf_P(PSTR("receive_data\n\r"));
	// send start condition:
	if (!twi_start()){
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		return 0;
	}
	TWDR = (address<<1) | 1;
	// send read request:
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
	//twi_wait();
	//printf_P(PSTR("TWSR = %x\n\r", TWSR));
	switch(TWSR){
		case 0x38:
			// Arbitration lost in SLA+R or NOT ACK bit

			// 2-wire Serial Bus will be released and not addressed
			// Slave mode will be entered
			TWCR = (1<<TWINT)|(1<<TWEN);
			printf_P(PSTR("error 0x38\n\r"));
			return 0;
			break;
		case 0x40:
			// SLA+R has been transmitted;
			// ACK has been received

			// Data byte will be received and ACK will be returned
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
			break;
		case 0x48:
			// SLA+R has been transmitted;
			// NOT ACK has been received
			
			// STOP condition will be transmitted and TWSTO Flag
			// will be reset
			TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

			printf_P(PSTR("error 0x48\n\r"));
			return 0;
			break;
		case 0xf8:
			// No relevant state information
			// available; TWINT = “0”
			// FIXME: this reaction causes a bus error
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
			printf_P(PSTR("no new state\n\r"));
			return 0;
			break;
	}
	// wait max 800ms, as measurement should be finished by then
	twi_wait_timeout(800);
	//twi_wait();
	//printf_P(PSTR("TWSR = %x\n\r", TWSR));
	while(TWSR == 0x50 && len > 0){
		//printf_P(PSTR("byte received!\n\r"));
		*buffer = TWDR;
		buffer++;
		len--;
		if (len == 1){
			TWCR = (1<<TWINT)|(1<<TWEN);
		}else{
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
		}
		if(!twi_wait_timeout(5)){
			printf_P(PSTR("timeout\n\r"));
		}
		//printf_P(PSTR("TWSR = %x\n\r", TWSR));
	}
	*buffer = TWDR;
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	return (len == 1);
	
	
}
