#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"

#include "board_support.h"

#define SLA 0x77
#define CRC8 49
// sensors connected at:
// PC0 - PCINT8
// PB0 - PCINT0
// PD7 - PCINT23

uint8_t send_buffer[8];
uint8_t stable_data[8];
uint8_t bufferpointer;
uint8_t icount;

#ifdef DEBUG
uint8_t s = 0;
uint8_t tcrita;
uint8_t tcritb;
uint8_t times[30];
uint8_t cf = 0;
uint8_t timesr[30];
uint8_t cr = 0;
#endif

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

	return cels;

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

void twi_init(){
  // set slave address
  // do NOT listen to general call
  TWAR = (SLA << 1);
  TWCR = (1<<TWEA) | (1<<TWEN);
}

void handle_communications(){
  if(TWCR & (1<<TWINT)){
    //TWI interrupt
		printf("TWSR = %x\n\r", TWSR);
    switch (TWSR){
      case 0xa8:
        // own address received, ack has been returned:
				// prepare data to send:
				send_buffer[0] = stable_data[0];
				send_buffer[1] = stable_data[1];
				send_buffer[2] = stable_data[2];
				send_buffer[3] = stable_data[3];
				send_buffer[4] = stable_data[4];
				send_buffer[5] = stable_data[5];
				send_buffer[6] = stable_data[6];
				send_buffer[7] = stable_data[7];
				bufferpointer = 1;
        TWDR = send_buffer[0]; //data :)
        TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
				break;
      case 0xb8:
				// data byte has been transmitted, ack has been received
				// FIXME: this allows master to cause buffer overflow read :P
        TWDR = send_buffer[bufferpointer]; //data :)
				bufferpointer++;
        TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
        break;
      case 0xc0:
				// data byte has been transmitted NACK has been received
        TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWINT);
        break;
			case 0xc8:
				// last data byte in transmission was transmitted, ACK has been received
        break;
			case 0x00:
				printf("bus error\n\r");
        TWCR = (1<< TWEA) | (1<<TWSTO) | (1<<TWEN) | (1<<TWINT);
      
    }

  }
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
    if (!verifyCRC(data, 4)){
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

void printarray(uint8_t * arr, uint8_t len){
  uint8_t i;
  for (i=0;i<len;i++){
    printf(" %x ", arr[len-1-i]);
  }
  printf("\n\r");
}

void loop(){
	uint8_t i;
	//printf("---\n\r");
	uint16_t cels1;
	uint16_t cels2;
	// start meassurement:
	//meassure_start_bank1();
	//_delay_ms(150);
	//meassure_stop_bank1();

#ifdef DEBUG
	cf = 0;
	cr = 0;
#endif
  icount = 0;
	sensor_pin_mask1 = (1<< PC2);
	nsensor_pin_mask1 = ~(1<< PC2);
	sensor_pin_mask2 = (1<< PD3);
	nsensor_pin_mask2 = ~(1<< PD3);
	meassure_start_bank1();
	meassure_start_bank2();
	for(i=0;i<12;i++){
		handle_communications();
		_delay_ms(15);
	}
	uint8_t res_m1 = meassure_stop_bank1();
  //printf("icount = %u\n\r", icount);
	uint8_t res_m2 = meassure_stop_bank2();
#ifdef DEBUG
	printf("cf = %d\n\r", cf);
	printf("crita = %d critb = %d\n\r", tcrita, tcritb);
	printf("Tr%d = %d\n\r", 0, timesr[0]);
	printf("Tf%d = %d\n\r", 1, times[1]);
	printf("Tr%d = %d\n\r", 10, timesr[10]);
	printf("Tf%d = %d\n\r", 11, times[11]);
#endif
	
	stable_data[0] = bytearr_bank1[0];
	stable_data[1] = bytearr_bank1[1];
	stable_data[2] = bytearr_bank1[2];
	stable_data[4] = bytearr_bank2[0];
	stable_data[5] = bytearr_bank2[1];
	stable_data[6] = bytearr_bank2[2];

  //printarray(bytearr_bank1, 5);
  //printarray(bytearr_bank2, 5);

	printf("Bank1: ");
	if(res_m1){
		interpret(bytearr_bank1);
	}else{
		printf("T = xxxx");
	}
	printf("   Bank2: ");
	if(res_m2){
		interpret(bytearr_bank2);
	}else{
		printf("T = xxxx");
	}
  printf("\n\r");
	//cels1 = analyze(bytearr_bank1);
	//cels2 = analyze(bytearr_bank2);

	//printf("bank1: %d  bank2: %d\n\r", cels1, cels2);
	for(i=0;i<50;i++){
		handle_communications();
		_delay_ms(10);
	}
}

void io_init(void){
	LED1_DDR |= (1<<LED1);
	LED2_DDR |= (1<<LED2);
	LED3_DDR |= (1<<LED3);
	LED4_DDR |= (1<<LED4);

	SW1_DDR &= ~(1<<SW1);

	LED1_PORT |= (1<< LED1);
	LED2_PORT |= (1<< LED2);
	LED3_PORT |= (1<< LED3);
	LED4_PORT |= (1<< LED4);
}

int main (void)
{
	static FILE usart_stdout = FDEV_SETUP_STREAM( mputc, 0, _FDEV_SETUP_WRITE);
	stdout = &usart_stdout;

	io_init();

	uart_init();
	sei();
	twi_init();

	int_init1();
	int_init2();

	printf("Controller started\n\r");
	while (1) {
		loop();
		LED1_PORT ^= (1<<LED1);
	}
}
