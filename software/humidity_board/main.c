#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#define DELAY_AFTER_MR_MS 55
#define DELAY_BEFORE_DF_US 20
#define START_TIMER TCCR0B |= (1<< CS01);// 8mHz/8
//#define START_TIMER TCCR0B |= (1<< CS01 | 1<<CS00);// 8mHz/64  
//#define START_TIMER TCCR0B |= (1<< CS02 | 1<<CS00);// 8mHz/1024
#define STOP_TIMER TCCR0B = 0; //disable timer
#define SLA 0x28
#define CMODE 7
#define STALE 6
#define TSTART_US 64
#define TLONG_US 96
#define TSHORT_US 32
#define CRC8 49
//set debug module
//#define DEBUG

uint8_t capH=0;
uint8_t capL=0;
uint8_t tempH=0;
uint8_t tempL=0;
uint8_t crc=0;
#ifdef DEBUG
uint8_t crc_verified=0xff;
uint8_t cap=0xff;
uint8_t temp=0xff;
#endif
int8_t counter_sent = 0;
volatile int8_t sending = 0;

uint8_t waitUntilFinished(uint8_t status){
  while(!(TWCR & (1 << TWINT))){
    // FIXME do break 
    asm("nop");
     //printf("TWSR is %x\n\r", TWSR);
    // sorry, i'm busy wating!!
  }
  if (TWSR == status){
    return 1;  
  }
  return 0;
}

void  requestMeasuring(){
#ifdef DEBUG
  printf("Measurement Request\n\r");
#endif
  /* Start condition*/
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  if (!waitUntilFinished(0x8)){
    return;  
  }
  /* SLA + W (bit 0)*/
  TWDR = (SLA << 1);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  /* TWSR = 0x18 means SLA+W has been transmitted; ACK has been received*/
  if (!waitUntilFinished(0x18)){
    return;
  }
  /* Stop condition*/
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
#ifdef DEBUG
  printf("Required\n\r");
#endif
}

uint8_t readByte(uint8_t ack){
  TWCR = ((1 << TWINT) | (ack << TWEA) | (1 << TWEN));
  // TWSR = 0x50 means Data byte has been received; ACK has been returned
  if (!waitUntilFinished(0x50)){
    return 0;
  }
  return TWDR;
}

void fetchData(){
#ifdef DEBUG
  printf("Data Fetch\n\r");
#endif
  /* Start condition*/
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  if (!waitUntilFinished(0x8)){
    return;  
  }
  /* SLA + R (bit 1)*/
  TWDR = (SLA << 1) | (1 << 0);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  /* TWSR = 0x40 means SLA+R has been transmitted; ACK has been received*/
  if (!waitUntilFinished(0x40)){
    return;
  }

  capH = readByte(1);
  capL = readByte(1);
  tempH = readByte(1);
  tempL = readByte(0);

  /* Stop condition*/
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
#ifdef DEBUG
  printf("Fetched\n\r");
#endif
}

void computeByteCRC(uint8_t *result, uint8_t byte) {
  int8_t index = 7;
  for(index=7; index >= 0; index--){
    if(*result & (1<<7)){
      *result = *result << 1;
      *result |= ((byte >> index) & 1);
      *result ^= CRC8;
    } else  {
      *result = *result << 1;
      *result |= ((byte >> index) & 1);
    }
  }
}

void computeCRC(){
  crc = capH;
  computeByteCRC(&crc, capL);
  computeByteCRC(&crc, tempH);
  computeByteCRC(&crc, tempL);
  computeByteCRC(&crc, 0);
}

#ifdef DEBUG
void verifyCRC(){
  crc_verified = capH;
  computeByteCRC(&crc_verified, capL);
  computeByteCRC(&crc_verified, tempH);
  computeByteCRC(&crc_verified, tempL);
  computeByteCRC(&crc_verified, crc);
  printf("Verified Result is %x\n\r", crc_verified);
}
#endif

// interrupt handler for output compare register A
ISR(TIMER0_COMPA_vect){
  if (counter_sent >=0){
    if(counter_sent < 79){
      if(counter_sent%2 == 0){
        if(capH & (1<<7)){
        // send 1
          OCR0A = TSHORT_US;
        }else{
        // send 0
          OCR0A = TLONG_US;
        }
      } else { 
        if(capH & (1<<7)){
        // send 1
          OCR0A = TLONG_US;
        }else{
        // send 0
          OCR0A = TSHORT_US;
        }
        // Roll data 
        crc = crc << 1;
        // use "rol" to shift the other two bits with carry
        asm("rol %0" : "=r" (tempL) : "0" (tempL));
        asm("rol %0" : "=r" (tempH) : "0" (tempH));
        asm("rol %0" : "=r" (capL) : "0" (capL));
        asm("rol %0" : "=r" (capH) : "0" (capH));
      }
    } else{
      STOP_TIMER
      // Disable timer output compare match A interrupt when sending bits
      TIMSK0 &= ~( 1 << OCIE0A ); 
      // Disable clear timer TCNT0 on compare match register A: OCR0A
      TCCR0A &= ~(1 << WGM01); 
      sending = 0;
    }
  }
  counter_sent++;
}

void convertToZAC(){
#ifdef DEBUG
  printf("Start sending: capH = %x\n\r", capH);
#endif
  // Set OC0A at the beginning
  TCCR0A |= ((1 << COM0A0) | (1 << COM0A1)); 
  TCCR0B |= (1 << FOC0A);

  // prepare for sending START bit
  TCNT0 = 0;
  OCR0A = TSTART_US;
  counter_sent = -2;

	// Enable timer output compare match A interrupt when sending bits
  TIMSK0 |= ( 1 << OCIE0A ); 
  // Enable clear timer TCNT0 on compare match register A: OCR0A
  TCCR0A |= (1 << WGM01); 
  // Enable Toggle OC0A on compare match
  TCCR0A &= ~(1 << COM0A1); 
  TCCR0A |= (1 << COM0A0); 
  // Flag of sending
  sending = 1;
  // Timer init 
  START_TIMER 
  while(sending){
    // busy wating until finishing sending
    asm("nop");
  }
  // Set OC0A at the end 
  TCCR0A |= ((1 << COM0A0) | (1 << COM0A1)); 
  TCCR0B |= (1 << FOC0A);
}

void loop(){
  uint8_t counter=0;
  requestMeasuring();
  // measurement will be ready after 50...60ms (this value was aquired by experimental meassurement).
  _delay_ms(DELAY_AFTER_MR_MS);
  counter=0;
  do{
    _delay_us(DELAY_BEFORE_DF_US);
    counter ++;
    fetchData();
    /* Do datafetch until the stale bit is 0 
     * If more than 100 times, give up */
  }while((capH & ( (1 << STALE))) && counter <100 );
#ifdef DEBUG
  printf("Fetching counter is %d\n\r", counter);
#endif
  // remove status bits
  capH = capH & 0x3f;
  
  computeCRC();
#ifdef DEBUG
  printf("capH = %x\n\r", capH);
  printf("capL = %x\n\r", capL);
  printf("tempH = %x\n\r", tempH);
  printf("tempL = %x\n\r", tempL);
  printf("crc=%x\n\r", crc);

  // converted roughly for test
  cap = ((capH*3) >> 1) + (capH >>4);
  temp = (tempH >> 1) + (tempH >> 3) + (tempH >> 6) ;

  printf("converted cap = %u\n\r", cap);
  printf("converted temp = %u - 40 = %d\n\r", temp, temp-40);

  verifyCRC();
#endif

  convertToZAC();
}

int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	uart_init();
	sei();

  // init registers for i2c
  // Fscl = CPU clock frequency/(16 + 2(TWBR))
  // Set Fscl 50 kHz
  // TWBR = 72; 
  // Set Fscl 100 kHz
  TWBR = 32; 
  // Set Fscl 200 kHz
  // TWBR = 12; 

  printf("Start\n\r");
	DDRD |= (1<<PD6);
  _delay_ms(1000);
  // Set internal pull-ups
  PORTC |= (1<<PC4);
  PORTC |= (1<<PC5);

  while(1){
    loop();
  }
}
void led0(){
  // TODO debug, LED blink
  _delay_ms(500);
  PORTC = PORTC ^ (1<<PC0);
  _delay_ms(500);
  PORTC = PORTC ^ (1<<PC0);
}
void led1(){
  // TODO debug, LED blink
  _delay_ms(500);
  PORTC = PORTC ^ (1<<PC1);
  _delay_ms(500);
  PORTC = PORTC ^ (1<<PC1);
}
