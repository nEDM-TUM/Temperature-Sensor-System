#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

#define SLA 0x77

uint8_t check_parity(uint8_t value, uint8_t parity){
	for (;value!=0;value = value >> 1){
		parity ^= value & 0x1;
	}
	return parity == 0;
}


uint16_t analyze(uint8_t * buf){
	uint16_t result;
	uint8_t resth = ((buf[2]<<5) | (buf[1]>>3));
	uint8_t restl = ((buf[1]<<7) | (buf[0]>>1));
	if (!check_parity(restl, buf[0] & 0x1) ){
		printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (buf[1]>>2) & 0x1 ) ){
		printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		printf("FORMAT ERROR\n\r");
	}
  //result = (((buf[2]<<5) | (buf[1]>>3)) <<8) | ((buf[1]<<7) | (buf[0]>>1));
  result =( resth <<8)|restl;
  uint16_t cels = ((result * 25)>>8)*35-1000;

	return cels;

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

	uint8_t received[8];

	receive_data(received,8);
	uint8_t i;
	for(i=0;i<8;i++){
		printf("r%d = %x, ", i, received[i]);
	}
	uint16_t cels1, cels2;
	cels1 = analyze(received);
	cels2 = analyze(received + 4);

	printf("\n\r");
	printf("bank1: %d  bank2: %d\n\r", cels1, cels2);

	//printf("hello\n\r");
	_delay_ms(500);
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
