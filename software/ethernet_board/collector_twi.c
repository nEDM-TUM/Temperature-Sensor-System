#include "collector_twi.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>

uint8_t check_parity(uint8_t value, uint8_t parity){
	for (;value!=0;value = value >> 1){
		parity ^= value & 0x1;
	}
	return parity == 0;
}

int16_t analyze(uint8_t * buf){
	uint16_t result;
	uint8_t err = 0;
	uint8_t resth = ((buf[2]<<5) | (buf[3]>>3));
	uint8_t restl = ((buf[3]<<7) | (buf[4]>>1));
	if (!check_parity(restl, buf[4] & 0x1) ){
		err = 1;
		//printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (buf[3]>>2) & 0x1 ) ){
		err = 1;
		//printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		err = 1;
		//printf("FORMAT ERROR\n\r");
	}
  result =( resth <<8)|restl;

  int32_t result32 = (int32_t)(result);

  int32_t cels = result32*100L*70L/2047L - 1000L;

	if(err){
		return 0;
	}else{
		return (int16_t)cels;
	}

}

int16_t analyze_hum_temp(uint8_t * buf){
	uint16_t data;
	int32_t data32;
	int32_t result;
	uint8_t tempH = buf[2];
	uint8_t tempL = buf[3];
  data = ((tempH<<6) | (tempL>>2));
  data32 = (int32_t)(data);
  result = data32*16500L;
  result = (result >> 14) - 4000L;
	return(int16_t)result;
}

int16_t analyze_hum_hum(uint8_t * buf){
	uint16_t data;
	int32_t data32;
	int32_t result;
	uint8_t capH = buf[0] & ~((1<<7)|(1<<6));
	uint8_t capL = buf[1];
  data= (capH << 8) | capL;
  data32 = (int32_t)(data);
  result = data32*10000L;
  result = result >> 14;
	return(int16_t)result;
}

void twi_interpret(uint8_t * data){
	if (!(data[0] & (1<<7))){
		//printf("received: \n\r");
		//printarray(data, 5);
		//printf("\n\r");
		// this is a humidity sensor
    // check crc checksum:
		//printf("verify crc...\n\r");
    if (twi_computeCRC(data, 4, data[4])!=0){
      printf("CRC error\n\r");
    }
		//printf("done\n\r");
		int16_t cels = analyze_hum_temp(data);
		int16_t hum = analyze_hum_hum(data);
		printf(" T = %d, H = %d", cels, hum);

	}else{
		// this is a temperature sensor
		int16_t cels = analyze(data);
		printf("T = %d", cels);
	}
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
  //printf("\t\t\t!!!Result is %x\n\r", result);
  return result;
}

uint8_t twi_wait(void){
	while(!(TWCR & (1<<TWINT))){
		//printf("wait: TWSR = %x\n\r", TWSR);
		// wait for interrupt
	}
	return 1;
}
uint8_t twi_wait_timeout(uint16_t milliseconds){
	uint16_t i;
	i = 0;
	while(!(TWCR & (1<<TWINT))){
		_delay_ms(1);
		if (i>milliseconds){
			return 0;
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
			if(!twi_wait_timeout(5)){
				printf("scan timeout\n\r");
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
		}else{
			printf("scan error\n\r");
		}

	}
	return n;
}

uint8_t twi_start(){
	//do{
		//printf("sending start\n\r");
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
		_delay_us(200);
		TWBR = 20;
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
		twi_wait_timeout(5);
		//printf("got interrupt\n\r");
	//}while(TWSR!=0x08);
	if (TWSR != 0x08){
		printf("start error: %u\n\r", TWSR);
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
	//printf("start_measurement\n\r");
	if (!twi_start()){
		TWCR = (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
		printf("could not send start\n\r");
		return 0;
	}
	//printf("send SLA+W\n\r");
	// send SLA + W
	TWDR = (addr<<1);
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
	//printf("TWSR = %x\n\r", TWSR);
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
					printf("error: 0x30\n\r");
					return 0;
					break;
				default:
					//TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
					printf("bus error2: tate = %x\n\r", TWSR);
					return 0;
					break;
			}
			break;
		case 0x20:
			// SLA+W has been transmitted;
			// NOT ACK has been received
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
			printf("error: 0x20\n\r");
			return 0;
			break;
		case 0x38:
			// Arbitration lost in SLA+W or data bytes

			// 2-wire Serial Bus will be released and not addressed
			// Slave mode entered
			TWCR = (1<<TWINT) | (1<<TWEN);
			printf("error: 0x38\n\r");
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
			printf("error: 0xf8, no new state (timeout)\n\r");
			return 0;
			break;
		default:
			printf("bus error: state = %x\n\r", TWSR);
			return 0;
			break;
	}
	
	
	

}

uint8_t twi_set_address(uint8_t addr, uint8_t new_addr){	
  if(!twi_send_cmd(addr, CMD_SET_ADDRESS)){
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return 0;
  }
  TWDR = new_addr;
	TWCR = (1<<TWINT) | (1<<TWEN);
	twi_wait_timeout(5);
  if(TWSR == 0x28){
    return 1;
  }else{
		TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
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

uint8_t twi_receive_data(uint8_t address, uint8_t * buffer, uint8_t len){
	//printf("receive_data\n\r");
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
	//printf("TWSR = %x\n\r", TWSR);
	switch(TWSR){
		case 0x38:
			// Arbitration lost in SLA+R or NOT ACK bit

			// 2-wire Serial Bus will be released and not addressed
			// Slave mode will be entered
			TWCR = (1<<TWINT)|(1<<TWEN);
			printf("error 0x38\n\r");
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

			printf("error 0x48\n\r");
			return 0;
			break;
		case 0xf8:
			// No relevant state information
			// available; TWINT = “0”
			// FIXME: this reaction causes a bus error
			TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
			printf("no new state\n\r");
			return 0;
			break;
	}
	// wait max 800ms, as measurement should be finished by then
	twi_wait_timeout(800);
	//twi_wait();
	//printf("TWSR = %x\n\r", TWSR);
	while(TWSR == 0x50 && len > 0){
		//printf("byte received!\n\r");
		*buffer = TWDR;
		buffer++;
		len--;
		if (len == 1){
			TWCR = (1<<TWINT)|(1<<TWEN);
		}else{
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
		}
		if(!twi_wait_timeout(5)){
			printf("timeout\n\r");
		}
		//printf("TWSR = %x\n\r", TWSR);
	}
	*buffer = TWDR;
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
	//printf("len = %d\n\r", len);
	return (len == 1);
	
	
}
