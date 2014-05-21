#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

#define SLA 0x77
#define CRC8 49

uint8_t check_parity(uint8_t value, uint8_t parity){
	for (;value!=0;value = value >> 1){
		parity ^= value & 0x1;
	}
	return parity == 0;
}


uint16_t analyze(uint8_t * buf){
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
  uint16_t cels = ((result * 25)>>8)*35-1000;

	if(err){
		return 0;
	}else{
		return cels;
	}

}

uint16_t analyze_hum_temp(uint8_t * buf){
	uint8_t tempH = buf[2];
	uint8_t tempL = buf[1];
	return (tempH >> 1) + (tempH >> 3) + (tempH >> 6) - 40;
}

uint16_t analyze_hum_hum(uint8_t * buf){
	uint8_t capH = buf[4] & ~((1<<7)|(1<<6));
	uint8_t capL = buf[3];
	return ((capH*3) >> 1) + (capH >>4);
}

uint8_t verifyCRC(uint8_t * data, int8_t len){
  uint8_t result = data[len - 1];
  int8_t i;
  for (i=len-2; i>=0; i--){
    int8_t index;
    for(index=7; index >= 0; index--){
      if(result & (1<<7)){
        result = result << 1;
        result |= ((data[i] >> index) & 1);
        result ^= CRC8;
      } else  {
        result = result << 1;
        result |= ((data[i] >> index) & 1);
      }
    }
  }
  printf("\t\t\t!!!Result is %x\n\r", result);
  return result == 0;
}

void interpret(uint8_t * data){
	if (!(data[4] & (1<<7))){
		// this is a humidity sensor
    // check crc checksum:
		//printf("verify crc...\n\r");
    if (!verifyCRC(data, 5)){
      printf("CRC error\n\r");
    }
		//printf("done\n\r");
		uint16_t cels = analyze_hum_temp(data);
		uint16_t hum = analyze_hum_hum(data);
		printf(" T = %u, H = %u", cels, hum);

	}else{
		// this is a temperature sensor
		uint16_t cels = analyze(data);
		printf("T = %u", cels);

	}

}

void receive_data(uint8_t * buffer, uint8_t len){
	// send start condition:
	TWBR = 20;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))){
		// wait for interrupt
	}
	TWDR = (SLA<<1) | 1;
	// send read request:
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))){
		// wait for interrupt
	}
	//printf("read request sent!\n\r");
	//printf("TWSR = %x\n\r", TWSR);
	switch(TWSR){
		case 0x38:
			TWCR = (1<<TWINT)|(1<<TWEN);
			break;
		case 0x40:
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
			break;
		case 0x48:
			TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
			break;
	}
	while(!(TWCR & (1<<TWINT))){
		// wait for interrupt
	}
	while(TWSR == 0x50 && len > 0){
		//printf("byte received!\n\r");
		*buffer = TWDR;
		buffer++;
		len--;
		if (len == 0){
			TWCR = (1<<TWINT)|(1<<TWEN);
		}else{
			TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
		}
		while(!(TWCR & (1<<TWINT))){
			// wait for interrupt
		}
	}
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

}

void loop(){
	PORTD ^= (1<<PD5);

	uint8_t received[8][5];

	receive_data(received[0],40);

	uint8_t s;
	for(s = 0; s<8; s++){
		printf("P%u: ", s+1);
		interpret(received[s]);
		printf(" | ");
	}
	printf("\n\r");

	//printf("hello\n\r");
	_delay_ms(800);
}


int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;


	uart_init();
	sei();

	DDRD = (1<<PD5);

	printf("Controller started\n\r");
	while (1) {
		loop();
	}
}
