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
uint8_t tcrit = 0xff;
uint8_t bitcount = 255;
uint8_t parity;
uint8_t resparity;
uint32_t res4 = 255;

uint16_t result;

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
	if (tcrit != 0xff){
		// we have seen a start bit AND are finished with transmission
		// => stop measurement
		PCICR &= ~(1<< PCIE1); //disable interrupt for PCINT[14...8]
		// if already interrupts are there clear them
		// this is done by writing 1 to it
		PCIFR |= (1<< PCIF1);

	}
	PORTB = PORTB ^ (1<<PB1);
	//PORTB |= (1<<PB1);
}
// Interrupt handler of PCIE1 (PCINIT[14...8]!!!)
// IDEA: use OCRnA for tcrit storage to reduce stack usage
ISR(PCINT1_vect){
	//PORTB |= (1<<PB1);
	//PORTB = PORTB ^ (1<<PB1);
  // Read timer 2
	uint8_t tval = TCNT2;
  // Reset timer 2
	TCNT2 = 0;
  
	// this should filter out vibrations on the wire
	if ((tval < 2)  && (TCCR2B  == (1<<CS22))){
		printf("INSTABILITY!\n\r");
		return;
	}
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
		if(lowtime>tval){
			if(lowtime-tval < 2){
				tcrit = tval;
			}
		}else{
			if(tval-lowtime < 2){
				tcrit = tval;
			}
		}
	}
}

void interrupt_init(){
	// enable interrupt will be done when starting measurement
	//PCICR = (1<< PCIE1); //enable interrupt for PCINT[14...8]
	PCMSK1 = (1<< PCINT8); // PCINT8 -> PC0
  // Attention: Timer 0 is used by ardunio core
	TIMSK2 = (1<<TOIE2); // enable overflow interript for timer2
  // Timer init (reset to default)
	TCCR2A = 0;
	TCCR2B = 0;
}

uint8_t check_parity(uint8_t value, uint8_t parity){
	for (;value!=0;value = value >> 1){
		parity ^= value & 0x1;
	}
	return parity == 0;
}

void loop(){
	//printf("---\n\r");
	
	// start meassurement:
	// initialize tcrit (used to determine if measurement was successful)
	tcrit = 0xff;
	TCNT2 = 0;
	lowtime = 0xff;
	//enable interrupt for PCINT[14...8]
	PCICR = (1<< PCIE1);
	// wait >100ms for meassurement to complete
	// there should not happen too many interrupts, as they extend _delay_ms
	_delay_ms(200);
	if(PCICR & (1<< PCIE1)){
		// interrupt was still enabled
		// -> meassurement was not successful
		// disable interrupts now (to be in consistent state)
		PCICR &= ~(1<< PCIE1);
		// return error:
		bytea =0xff;
		byteb =0xff;
		bytec =0xff;
		printf("ERROR Meassurement not complete\n\r");
	}



	uint8_t ra,rb,rc;
	ra = bytea;
	rb = byteb;
	rc = bytec;
	uint8_t resth = ((ra<<5) | (rb>>3));
	uint8_t restl = ((rb<<7) | (rc>>1));

	if (!check_parity(restl, bytec & 0x1) ){
		printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (byteb>>2) & 0x1 ) ){
		printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		printf("FORMAT ERROR\n\r");
	}
	//result = (((ra<<5) | (rb>>3)) <<8) | ((rb<<7) | (rc>>1));
	result =( resth <<8)|restl;
	uint16_t cels = ((result * 25)>>8)*35-1000;
	//printf("ra: %x, rb: %x rc: %x\n\r", ra, rb, rc);
	//printf("resth: %x restl: %x\n\r", resth, restl);
	//printf("result: %x\n\r", result);
	//printf("cels: %u\n\r", cels);

	_delay_ms(500);
}


int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	DDRB = (1<<PB1);
	interrupt_init();

	uart_init();
	sei();

	printf("Controller started\n\r");
	while (1) {
		loop();
	}
}
