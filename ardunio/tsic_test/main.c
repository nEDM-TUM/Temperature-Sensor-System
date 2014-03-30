#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

// sensors connected at:
// PC0 - PCINT8
// PB0 - PCINT0
// PD7 - PCINT23

uint8_t lowtime = 255;
uint8_t bytea = 255;
uint8_t byteb = 255;
uint8_t bytec = 255;
uint8_t tcrit = 0xff;

uint16_t result;

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
	// we use the fact, that TSIC always sends 5 zeroes at the beginning
	// to determine, if the transmission was complete
	// as it might happen, that we start meassuring just before the
	// second start bit. We then have to wait for the next transmission
	if ((tcrit != 0xff) && (!(bytea & (1<<2))) ){
		// we have seen a start bit AND are finished with transmission
		// => stop measurement
		// disable interrupt for this pin
		// -> unmask:
		PCMSK1 &= ~(1<< PCINT8);
		// TODO: check, if we have to clear possibly arrived interrupts
		// this this would be done by writing 1 to the corresponding bit:
		// PCIFR |= (1<< PCIF1);

	}else{
		// we will receive the rest of the data in the next iteration
		// hence we have to clean up the buffer, as we have to have a defined state
		// to distinguish between humidity and temperature sensor
		bytea = 0xff;
		byteb = 0xff;
		bytec = 0xff;
	}
	PORTB = PORTB ^ (1<<PB1);
}

// Interrupt handler of PCIE1 (PCINIT[14...8]!!!)
// IDEA: use OCRnA/B for tcrit / tval storage to reduce stack usage
ISR(PCINT1_vect){
  // Read timer 2
	uint8_t tval = TCNT2;
  // Reset timer 2
	TCNT2 = 0;
  
	// this should filter out vibrations on the wire
	// FIXME: check if we need this. I was not able to construct
	// a situation, where this was called in a lab environment.
	// But maybe with long wires we might have spikes.
	// We need to check, if timer is already running,
	// as this might be the first interrupt, where
	// tval is 0 by default.
	// TODO: check if we can change start value for TCNT2 to
	// 0xff, then the second check would not be needed.
	if ((tval < 2)  && (TCCR2B  == (1<<CS22))){
		//printf("INSTABILITY!\n\r");
		return;
	}

	// TODO:
	// can we start the timer already outside?
	// we are now checking if we have received enough bytes
	// on timeout, so a overflow would not hurt
	// but we might get a random value for the first interrupt
	// which might falsely be detected as a start bit.
	// It would also mess with the spike detector above.
	// as the first interrupt could be falsely detected
	// as a spike.
	// start timer 2: enable with prescaler 64
	TCCR2B = (1<<CS22);

	if(PINC & (1<< PC0)){
		// PC0 is 1 -> rising edge
		// this is the time, the signal was low:
		lowtime = tval;
		// TODO: Idea:
		// Try to directly use 16 bis or 32 bit integers. this might reduce
		// stack and register overhead
		if (tcrit != 0xff){
			// here we receive a byte.
			// we now know, that we have received a start bit in the past
			// shift all 3 bytes to the left:
			// shift first byte -> carry flag is the msb
			bytec = (bytec << 1);
			// use "rol" to shift the other two bits with carry
			asm("rol %0" : "=r" (byteb) : "0" (byteb));
			asm("rol %0" : "=r" (bytea) : "0" (bytea));
			// now we write the received bit:
			// if this (rising) edge happened before the critical time,
			// we extracted from the start bit, we are dealing with a '1',
			// else with a '0'
			bytec = bytec ^ (tval < tcrit);
		}
	}else{
		// PC0 is 0 -> falling edge
		// we check for a start bit:
		// A start bit should have a duty cycle of 50%
		// We also accept bits as start bits within a small mergin
		// If we have detected a start bit, we use the high time of the
		// bit as the critical time tcrit
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
	PCICR = (1<< PCIE1); //enable interrupt for PCINT[14...8]
	//PCMSK1 = (1<< PCINT8); // PCINT8 -> PC0
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
	// this will be shifted through and help us to determine
	// if we have received enough bits
	bytea = 0xff;
	byteb = 0xff;
	bytec = 0xff;
	// if in the meantime interrupts have arrived -> clear them
	PCIFR |= (1<< PCIF1);
	//enable interrupt for PCINT[14...8]
	PCMSK1 = (1<< PCINT8); // PCINT8 -> PC0
	//PCICR = (1<< PCIE1);
	// wait >100ms for meassurement to complete
	// there should not happen too many interrupts, as they extend _delay_ms
	_delay_ms(200);
	if(PCMSK1 & (1<< PCINT8)){
		// interrupt was still enabled
		// -> meassurement was not successful
		// disable interrupts now (to be in consistent state)
		PCMSK1 &= ~(1<< PCINT8); // PCINT8 -> PC0
		//PCICR &= ~(1<< PCIE1);
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
	uint8_t error = 0;
	if (!check_parity(restl, bytec & 0x1) ){
		error = 1;
		printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (byteb>>2) & 0x1 ) ){
		error = 1;
		printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		error = 1;
		printf("FORMAT ERROR\n\r");
	}
	if (error){
		printf("tcrit: %d\n\r", tcrit);
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
