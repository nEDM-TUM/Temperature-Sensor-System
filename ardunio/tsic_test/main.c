#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include "strfun.h"
#include "usart.h"

// sensors connected at:
// PC0 - PCINT8
// PB0 - PCINT0
// PD7 - PCINT23

uint8_t lowtime = 255;
uint8_t deb1 = 255;
uint8_t deb2 = 255;
uint8_t bytea = 255;
uint8_t byteb = 255;
uint8_t bytec = 255;
uint8_t resa = 255;
uint8_t resb = 255;
uint8_t resc = 255;
uint8_t tcrit;
uint8_t bitcount = 255;
uint8_t parity;
uint8_t resparity;
uint32_t res4 = 255;

uint16_t result;

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
	resa = bytea;
	resb = byteb;
	resc = bytec;
	PORTB = PORTB ^ (1<<PB1);
}
// Interrupt handler of PCIE1 (PCINIT[14...8]!!!)
ISR(PCINT1_vect){
  // Read timer 2
	uint8_t tval = TCNT2;
  // Reset timer 2
	TCNT2 = 0;
  
	if(PINC & (1<< PC0)){
		// PC0 is 1 -> rising edge
		lowtime = tval;
		bytec = (bytec << 1);
		// idea: use only 16 bit here, as most significant bits are not used
		// anyways
		// Also try to directly use 16 bis or 32 bit integers. this might reduce
		// stack and register overhead
		asm("rol %0" : "=r" (byteb) : "0" (byteb));
		asm("rol %0" : "=r" (bytea) : "0" (bytea));
		bytec = bytec ^ (tval < tcrit);
	}else{
		// PC0 is 0 -> falling edge
		TCCR2B = (1<<CS22); // start timer 2: enable with prescaler 64
    //// FIXME find appropriate range
		//if((lowtime < (tval + 3)) && (lowtime > (tval - 3))){
		//	//this was start bit :)
		//	tcrit = tval;
		//}
		// code below is bullshit???
		if(lowtime>tval){
			if(lowtime-tval < 3){
				tcrit = tval;
			}
		}else{
			if(tval-lowtime < 3){
				tcrit = tval;
			}
		}
	}
}

void interrupt_init(){
	PCICR = (1<< PCIE1); //enable interrupt for PCINT[14...8]
	PCMSK1 = (1<< PCINT8); // PCINT8 -> PC0
  // Attention: Timer 0 is used by ardunio core
	TIMSK2 = (1<<TOIE2); // enable overflow interript for timer2
  // Timer init (reset to default)
	TCCR2A = 0;
	TCCR2B = 0;
}

void loop(){

	uint8_t ra,rb,rc;
	cli();
	ra = resa;
	rb = resb;
	rc = resc;
	sei();
	uint8_t resth = ((ra<<5) | (rb>>3));
	uint8_t restl = ((rb<<7) | (rc>>1));
	//result = (((ra<<5) | (rb>>3)) <<8) | ((rb<<7) | (rc>>1));
	result =( resth <<8)|restl;
	printf("ra: %x, rb: %x rc: %x\n\r", ra, rb, rc);
	printf("resth: %x restl: %x\n\r", resth, restl);
	printf("result: %x\n\r", result);
	uint16_t cels = ((result * 25)>>8)*35-1000;
	printf("cels: %u\n\r", cels);

	_delay_ms(500);
}


int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	DDRB = (1<<PB1);
	interrupt_init();

	uart_init();

	printf("Controller started\n\r");
	while (1) {
		loop();
	}
}
