#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

// sensors connected at:
// PC0 - PCINT8
// PB0 - PCINT0
// PD7 - PCINT23

uint8_t lowtime1 = 255;
uint8_t bytearr_bank1[3];
uint8_t sensor_pin_mask1 = (1<<PC0);
uint8_t nsensor_pin_mask1 = ~(1<<PC0);
uint8_t lowtime2 = 255;
uint8_t bytearr_bank2[3];
uint8_t sensor_pin_mask2 = (1<<PB0);
uint8_t nsensor_pin_mask2 = ~(1<<PB0);
uint8_t s = 0;
uint8_t tcrita;
uint8_t tcritb;
uint8_t times[30];
uint8_t cf = 0;
uint8_t timesr[30];
uint8_t cr = 0;

#undef BANK
#define BANK 1
#include "tsic_template.c"
#undef BANK
#define BANK 2
#include "tsic_template.c"


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
  //if (resth != storeH){
    //printf("!!!!HIGHT bits are changed: 0x%x => 0x%x\n\r", storeH, resth);
    //printf("buf[2]: %x, buf[1]: %x buf[0]: %x\n\r", buf[2], buf[1], buf[0]);
    //printf("resth: %x restl: %x\n\r", resth, restl);
    //printf("result: %x\n\r", result);
    //printf("cels: %d\n\r", cels);
    //printf("0x%x%x cels (not exact): %d\n\r", resth, restl,(resth<<3)-7+(resth>>1)+(resth>>2)+(restl>>5));
    //printf("cels: %u\n\r", cels);
  //} else if(restl != storeL){
    //printf("Low bits are changed: 0x%x => 0x%x\n\r", storeL, restl);
  //}

	return cels;

}

void loop(){
	//printf("---\n\r");
	uint16_t cels1;
	uint16_t cels2;
	// start meassurement:
	//meassure_start_bank1();
	//_delay_ms(150);
	//meassure_stop_bank1();

	cf = 0;
	cr = 0;
	printf("start\n\r");
	meassure_start_bank2();
	_delay_ms(110);
	meassure_stop_bank2();
	printf("cf = %d\n\r", cf);
	printf("crita = %d critb = %d\n\r", tcrita, tcritb);
		printf("Tr%d = %d\n\r", 0, timesr[0]);
		printf("Tf%d = %d\n\r", 1, times[1]);
		printf("Tr%d = %d\n\r", 10, timesr[10]);
		printf("Tf%d = %d\n\r", 11, times[11]);
	//cels1 = analyze(bytearr_bank1);

	cels2 = analyze(bytearr_bank2);

	printf("bank1: %d  bank2: %d\n\r", cels1, cels2);
	_delay_ms(500);
}


int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;


	uart_init();
	sei();

	DDRB = (1<<PB1);
	int_init1();
	int_init2();

	printf("Controller started\n\r");
	while (1) {
		loop();
	}
}
