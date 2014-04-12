#include <avr/io.h>
#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include "strfun.h"
#include "usart.h"
#define DELAY_AFTER_MR_MS 55
#define DELAY_BEFORE_DF_US 20
#define START_TIMER TCCR0B = (1<< CS02 | 1 << CS00);
#define STOP_TIMER TCCR0B = 0; //disable timer
#define SLA 0x28
#define CMODE 7
#define STALE 6
#define TSTART 60
#define TLONG 90
#define TSHORT 30
uint8_t capH=0xff;
uint8_t capL=0xff;
uint8_t tempH=0xff;
uint8_t tempL=0xff;
// FIXME converted roughly for test
uint8_t cap=0xff;
uint8_t temp=0xff;
uint8_t counter=0;

void waitUntilFinished(){
  while(!(TWCR & (1 << TWINT))){
    asm("nop");
    // Sorry, I'm busy wating!!
  }
}
void waitUntil(){
  while(!(TWCR & (1 << TWINT))){
    printf("TWSR is %x\n\r", TWSR);
    asm("nop");
    // Sorry, I'm busy wating!!
  }
}
#define  MEASURINGREQUEST\
/*{*/\
  /*printf("Measurement Request\n\r");*/\
  /* Start condition*/\
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));\
  waitUntilFinished();\
  /*printf("TWCR out= %x\n\r", TWCR);*/\
  /*printf("TWSR= %x\n\r", TWSR);*/\
  if (TWSR != 0x8){\
    return;  \
  }\
  /*printf("Started\n\r");*/\
  /* SLA + W (bit 0)*/\
  TWDR = (SLA << 1);\
  TWCR = ((1 << TWINT) | (1 << TWEN));\
  waitUntilFinished();\
  /* TWSR = 0x18 means SLA+W has been transmitted; ACK has been received*/\
  if (TWSR != 0x18){\
    return;\
  }\
  /* Stop condition*/\
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));\
 /* printf("Required\n\r");*/\
  /* TODO what happend???*/\
  /* waitUntilFinished();*/\
/*}*/

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

#define DATAFETCH\
/*{*/\
  /* printf("Data Fetch\n\r");*/\
  /* Start condition*/\
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));\
  waitUntilFinished();\
  if (TWSR != 0x8){\
    return;  \
  }\
  /* SLA + R (bit 1)*/\
  TWDR = (SLA << 1) | (1 << 0);\
  TWCR = ((1 << TWINT) | (1 << TWEN));\
  waitUntilFinished();\
  /* TWSR = 0x40 means SLA+R has been transmitted; ACK has been received*/\
  if (TWSR != 0x40){\
    return;\
  }\
\
  capH = readByte(1);\
  capL = readByte(1);\
  tempH = readByte(1);\
  tempL = readByte(0);\
\
  /* Stop condition*/\
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));\
 /* printf("Fetched\n\r");*/\
/*}*/

#define SEND_START\
/*{*/\
  PORTC &= ~(1<<PC0);  \
  _delay_us(TSTART);\
  PORTC |= (1<<PC0);  \
  _delay_us(TSTART);\
  PORTC &= ~(1<<PC0);  \
/*}*/

#define SEND0\
/*{*/\
  _delay_us(TLONG);\
  PORTC |= (1<<PC0);  \
  _delay_us(TSHORT);\
/*}*/

#define SEND1\
/*{*/\
  _delay_us(TSHORT);\
  PORTC |= (1<<PC0);  \
  _delay_us(TLONG);\
/*}*/

#define SEND_FIRST_BYTE(byte) {\
  int8_t index = 0;\
  int8_t byte_copy = byte;\
  for(index=7; index >= 0; index--){\
    PORTC &= ~(1<<PC0);  \
    if(byte_copy & (1 << 7)){\
      SEND1\
    }else{\
      SEND0\
    }\
    byte_copy = (byte_copy<<1);\
  }\
}

void sendBYTE(uint8_t byte){
  int8_t index = 0;
  for(index=7; index >= 0; index--){
    PORTC &= ~(1<<PC0);
    if(byte & (1 << 7)){
      SEND1
    }else{
      SEND0
    }
    byte = (byte<<1);
  }
}

#define CONVERT_TO_ZAC\
/*{*/\
  SEND_START\
  SEND_FIRST_BYTE(capH)\
  sendBYTE(capL);\
  sendBYTE(tempH);\
  sendBYTE(tempL);\
/*}*/

void loop(){
  // TODO debug, LED blink
  //_delay_ms(500);
  //PORTD = PORTD ^ (1<<PD5);
  //_delay_ms(500);
  //PORTD = PORTD ^ (1<<PD5);
  MEASURINGREQUEST
    // measurement will be ready after 50...60ms (this value was aquired by experimental meassurement).
  _delay_ms(DELAY_AFTER_MR_MS);
  counter=0;
  do{
    _delay_us(DELAY_BEFORE_DF_US);
    counter ++;
    DATAFETCH
    /* Do datafetch until the stale bit is 0 
     * If more than 100 times, give up */
  }while((capH & ( (1 << STALE))) && counter <100 );
  //printf("Counter is %d\n\r", counter);
  capH = capH & 0x3f;
  CONVERT_TO_ZAC 
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
  TWBR = 32; 

  printf("Start\n\r");
  // PC0 as output port
	DDRC = (1<<PC0);
  PORTC |= (1<<PC0);
    // TODO debug, LED blink
	DDRD = (1<<PD5);
  PORTD = PORTD ^ (1<<PD5);
    _delay_ms(500);
  PORTD = PORTD ^ (1<<PD5);
    _delay_ms(500);
  PORTD = PORTD ^ (1<<PD5);
    _delay_ms(500);
  PORTD = PORTD ^ (1<<PD5);
    _delay_ms(500);
  PORTD = PORTD ^ (1<<PD5);

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
