#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
extern "C" {
#include "usart.h"
}
#include "configuration.h"

#define CMD_START_MEASUREMENT 1
#define CMD_SET_ADDRESS 2

#define SLA1 0x77
#define SLA2 0x78
#define CRC8 49

uint8_t check_parity(uint8_t value, uint8_t parity){
	for (;value!=0;value = value >> 1){
		parity ^= value & 0x1;
	}
	return parity == 0;
}

int16_t analyze(uint8_t * buf){
	uint16_t result;
	uint8_t err = 0;
	uint8_t resth = ((buf[2]<<5) | (buf[1]>>3));
	uint8_t restl = ((buf[1]<<7) | (buf[0]>>1));
	if (!check_parity(restl, buf[0] & 0x1) ){
		err = 1;
		//printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (buf[1]>>2) & 0x1 ) ){
		err = 1;
		//printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		err = 1;
		//printf("FORMAT ERROR\n\r");
	}
  //result = (((buf[2]<<5) | (buf[1]>>3)) <<8) | ((buf[1]<<7) | (buf[0]>>1));
  result =( resth <<8)|restl;
  //uint32_t result32 = (uint32_t)result;


  int32_t result32 = (int32_t)(result);
  //for (i =0;i<25;i++){
  //  result32 += result32;
  //}
  //result32 = result32 << 2;
  //result32 = result32 * 100UL * 70UL;
  //printf("3ff = %lu\n\r", result32/2047UL - 1000UL);
  //int32_t c100 = 100;
  //printf("3ff = %d\n\r", result32*c100);

  int32_t cels = result32*100L*70L/2047L - 1000L;

  //uint16_t cels = ((result * 25)>>8)*35-1000;

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
	uint8_t tempL = buf[1];
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
	uint8_t capH = buf[4] & ~((1<<7)|(1<<6));
	uint8_t capL = buf[3];
  data= (capH << 8) | capL;
  data32 = (int32_t)(data);
  result = data32*10000L;
  result = result >> 14;
	return(int16_t)result;
}


uint8_t computeCRC(uint8_t * data, uint8_t len, uint8_t crc){
  uint8_t result = data[0];
  uint8_t byte;
  int8_t i=1;
  int8_t index;
  while (1){
    if(i<len){
      byte = data[i];
    } else if(i==len){
      byte=crc;
    }else{
      break;
    }
    for(index=7; index >= 0; index--){
      if(result & (1<<7)){
        result = result << 1;
        result |= ((byte >> index) & 1);
        result ^= CRC8;
      } else  {
        result = result << 1;
        result |= ((byte >> index) & 1);
      }
    }
    i++;
  }
  //printf("\t\t\t!!!Result is %x\n\r", result);
  return result;
}

void interpret(uint8_t * data){
	if (!(data[4] & (1<<7))){
		// this is a humidity sensor
    // check crc checksum:
		//printf("verify crc...\n\r");
    if (computeCRC(data, 4,  data[4])!=0){
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
		return 0;
	}else{
		//printf("done\n\r");
		return 1;
	}
	//return twi_wait_timeout(5);
	//while(!(TWCR & (1<<TWINT)) || (TWSR != 0x08) ){
	//	// wait for interrupt
	//	// and right status code
	//}
}

uint8_t start_measurement(uint8_t addr){
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
			TWDR = CMD_START_MEASUREMENT;
			TWCR = (1<<TWINT)|(1<<TWEN);
			twi_wait_timeout(5);
			switch(TWSR){
				case 0x28:
					// Data byte has been transmitted;
					// ACK has been received
					TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
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

uint8_t receive_data(uint8_t address, uint8_t * buffer, uint8_t len){
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

// -- void receive_data(uint8_t address, uint8_t * buffer, uint8_t len){
// -- 	// send start condition:
// -- 	TWBR = 20;
// -- 	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
// -- 	while(!(TWCR & (1<<TWINT))){
// -- 		// wait for interrupt
// -- 	}
// -- 	TWDR = (address<<1) | 1;
// -- 	// send read request:
// -- 	TWCR = (1<<TWINT) | (1<<TWEN);
// -- 	while(!(TWCR & (1<<TWINT))){
// -- 		// wait for interrupt
// -- 	}
// -- 	//printf("read request sent!\n\r");
// -- 	//printf("TWSR = %x\n\r", TWSR);
// -- 	switch(TWSR){
// -- 		case 0x38:
// -- 			TWCR = (1<<TWINT)|(1<<TWEN);
// -- 			break;
// -- 		case 0x40:
// -- 			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
// -- 			break;
// -- 		case 0x48:
// -- 			TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
// -- 			break;
// -- 	}
// -- 	while(!(TWCR & (1<<TWINT))){
// -- 		// wait for interrupt
// -- 	}
// -- 	while(TWSR == 0x50 && len > 0){
// -- 		//printf("byte received!\n\r");
// -- 		*buffer = TWDR;
// -- 		buffer++;
// -- 		len--;
// -- 		if (len == 0){
// -- 			TWCR = (1<<TWINT)|(1<<TWEN);
// -- 		}else{
// -- 			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
// -- 		}
// -- 		while(!(TWCR & (1<<TWINT))){
// -- 			// wait for interrupt
// -- 		}
// -- 	}
// -- 	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
// -- 
// -- }
void printarray(uint8_t * arr, uint8_t len){
  uint8_t i;
  for (i=0;i<len;i++){
    printf(" %x ", arr[len-1-i]);
  }
  printf("\n\r");
}


void loop(){
	uint8_t s;
	PORTD ^= (1<<PD5);

	uint8_t received[8][5];

	uint8_t state;

	// -- printf("receive from 0x77..\n\r");
	// -- receive_data(SLA1, received[0],40);

	// -- uint8_t s;
	// -- for(s = 0; s<8; s++){
	// -- 	printf("P%u: ", s+1);
	// -- 	interpret(received[s]);
	// -- 	printf(" | ");
	// -- }
	// -- printf("\n\r");

	//printf("----\n\r");
	printf("----------\n\r");
	state = start_measurement(0x00);
	//state = start_measurement(SLA2);
	state = receive_data(SLA1, ((uint8_t*)received),40);
	printf("0x%x: ", SLA1);
	if (state){
		//printarray((uint8_t*)received, 40);
		for(s = 0; s<8; s++){
			printf("P%u: ", s+1);
			interpret(received[s]);
			printf(" | ");
		}
		printf("\n\r");
	}else{
		printf("receive interrupted\n\r");
	}
	state = receive_data(SLA2, ((uint8_t*)received),40);
	printf("\n\r");
	printf("0x%x: ", SLA2);
	if (state){
		//printarray((uint8_t*)received, 40);
		for(s = 0; s<8; s++){
			printf("P%u: ", s+1);
			interpret(received[s]);
			printf(" | ");
		}
		printf("\n\r");
	}else{
		printf("receive interrupted\n\r");
	}
	//printf("Measurement started..\n\r");
	//printf("receiving..\n\r");
	
	// -- if (state){
	// -- 	state = receive_data(SLA2, ((uint8_t*)received),40);
	// -- 	//state = 0;
	// -- 	if (state){
	// -- 		//printarray((uint8_t*)received, 40);
	// -- 		printf("\n\r");
	// -- 		for(s = 0; s<8; s++){
	// -- 			printf("P%u: ", s+1);
	// -- 			interpret(received[s]);
	// -- 			printf(" | ");
	// -- 		}
	// -- 		printf("\n\r");
	// -- 	}else{
	// -- 		printf("receive interrupted\n\r");
	// -- 	}
	// -- }else{
	// -- 	printf("device not found\n\r");
	// -- }

	//printf("hello\n\r");
}


int main (void)
{


	uart_init();
	sei();

	DDRD = (1<<PD5);

	printf("Controller started\n\r");
  //setupServerLib();
  //setupServer();
	while (1) {
		loop();
	}
}
