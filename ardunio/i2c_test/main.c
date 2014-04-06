#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include "strfun.h"
#include "usart.h"

#define START_TIMER TCCR0B = (1<< CS02 | 1 << CS00);
#define STOP_TIMER TCCR0B = 0; //disable timer
#define SLA 0x28
#define CMODE 7
#define STALE 6
uint8_t ready=1;
uint8_t capH=0xff;
uint8_t capL=0xff;
uint8_t tempH=0xff;
uint8_t tempL=0xff;
// FIXME converted roughly for test
uint8_t cap=0xff;
uint8_t temp=0xff;

void waitUntilFinished(){
  while(!(TWCR & (1 << TWINT))){
    asm("nop");
    // Sorry, I'm busy wating!!
  }
}
inline void mr(){
  printf("Measurement Request\n\r");
  // Start condition
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  waitUntilFinished();
  //printf("TWCR out= %x\n\r", TWCR);
  //printf("TWSR= %x\n\r", TWSR);
  if (TWSR != 0x8){
    return;  
  }
  //printf("Started\n\r");
  // SLA + W (bit 0)
  TWDR = (SLA << TWD1);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x18 means SLA+W has been transmitted; ACK has been received
  if (TWSR != 0x18){
    return;
  }
  // Stop condition
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
  printf("Required\n\r");
  // TODO what happend???
  // waitUntilFinished();
}

uint8_t readByte(uint8_t ack){
  //printf("TWDR SLA + R = 0x%x\n\r", TWDR);
  TWCR = ((1 << TWINT) | (ack << TWEA) | (1 << TWEN));
  //printf("TWDR already received ??? = 0x%x\n\r", TWDR);
  waitUntilFinished();
  // TWSR = 0x50 means Data byte has been received; ACK has been returned
  if (TWSR != 0x50){
    return 0xff;
  }
  //printf("Read finished\n\r");
  return TWDR;
}

inline void df(){
  // printf("Data Fetch\n\r");
  // Start condition
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  waitUntilFinished();
  if (TWSR != 0x8){
    return;  
  }
  // SLA + R (bit 1)
  TWDR = (SLA << TWD1) | (1 << TWD0);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x40 means SLA+R has been transmitted; ACK has been received
  if (TWSR != 0x40){
    return;
  }

  capH = readByte(1);
  capL = readByte(1);
  tempH = readByte(1);
  tempL = readByte(0);

  // Stop condition
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
  //printf("Fetched\n\r");
}

inline uint8_t verifyStatus(){
  //if((capH & ((1 << CMODE) | (1 << STALE))) == 0){
  if((capH & ( (1 << STALE))) == 0){
    return 1;
  }
  return 0;
}

loop(){
    // TODO debug, LED blink
    PORTB = PORTB ^ (1<<PB1);
    _delay_ms(500);
    mr();
    do{
      df();
    }while(!verifyStatus());
  
    capH = capH & 0x3f;
    printf("capH = %x\n\r", capH);
    printf("capL = %x\n\r", capL);
    printf("tempH = %x\n\r", tempH);
    printf("tempL = %x\n\r", tempL);

    cap = ((capH*3) >> 1) + (capH >>4);
    temp = (tempH >> 1) + (tempH >> 3) + (tempH >> 6);

    printf("converted cap = %u\n\r", cap);
    printf("converted temp = %u - 40 = %d\n\r", temp, temp-40);
}

int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	uart_init();
	sei();

  // init registers for i2c
  // Set Fscl 100 kHz
  TWBR = 72; 

  printf("Start\n\r");
    // TODO debug, LED blink
	DDRB = (1<<PB1);
  PORTB = PORTB ^ (1<<PB1);
    _delay_ms(500);
  PORTB = PORTB ^ (1<<PB1);
    _delay_ms(500);
  PORTB = PORTB ^ (1<<PB1);
    _delay_ms(500);
  PORTB = PORTB ^ (1<<PB1);
    _delay_ms(500);

	// enable timer overflow interrupt
	//TIMSK0 = ( 1 << TOIE0 ); 
  // Timer init 
	//TCCR0A = 0;
  //START_TIMER 
  //TCNT0 = 0;
  while(1){
    loop();
  }
}
