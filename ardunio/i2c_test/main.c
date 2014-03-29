#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <stdint.h>
//#include <stdlib.h>
#include <util/delay.h>
//#include "strfun.h"
#include "usart.h"

uint8_t capH=0;
uint8_t capL=0;
uint8_t temp=0;

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
  printf("TWCR out= %x\n\r", TWCR);
  printf("TWSR= %x\n\r", TWSR);
  if (TWSR != 0x8){
    return;  
  }
  printf("Started\n\r");
  // SLA + W (bit 0)
  TWDR = (0x28 << 1);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x18 means SLA+W has been transmitted; ACK has been received
  if (TWSR != 0x18){
    return;
  }
  printf("Required\n\r");
  // Stop condition
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));
}

inline uint8_t readByte(){
  TWCR = ((1 << TWINT) | (1 << TWEA) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x50 means Data byte has been received; ACK has been returned
  if (TWSR != 0x50){
    return;
  }
  printf("Read finished\n\r");
  return TWDR;
}

inline void df(){
  printf("Data Fetch\n\r");
  // Start condition
  TWCR = ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN));
  waitUntilFinished();
  if (TWSR != 0x8){
    return;  
  }
  // SLA + R (bit 1)
  TWDR = (0x28 << TWD1) | (1 << TWD0);
  TWCR = ((1 << TWINT) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x40 means SLA+R has been transmitted; ACK has been received
  if (TWSR != 0x40){
    return;
  }
  TWCR = ((1 << TWINT) | (1 << TWEA) | (1 << TWEN));
  waitUntilFinished();
  // TWSR = 0x50 means Data byte has been received; ACK has been returned
  if (TWSR != 0x50){
    return;
  }
  capH = readByte();
  capL = readByte();
  temp = readByte();

  // Stop condition
  TWCR = ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN));


}

int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	uart_init();

  // init registers for i2c
  // Set Fscl 250 kHz
  TWBR = 24; 

  // FIXME not needed !!! Enable TWI operation
  TWCR |= 1 << TWEN;
  // Enable Acknowledge bit
  TWCR |= 1 << TWEA;

	printf("Do I2C reading\n\r");
	while (1) {
		_delay_ms(500);
    // TODO  debug, LED blink
    PORTB = PORTB ^ (1<<PB1);
    mr();
    df();
    printf("capH = %x\n\r", capH);
    printf("capL = %x\n\r", capL);
    printf("temp = %x\n\r", temp);
	}
}
