#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart.h"
#include <inttypes.h>

#include "board_support.h"

#define SLA 0x78
#define CRC8 49
// sensors connected at:
// PC0 - PCINT8
// PB0 - PCINT0
// PD7 - PCINT23

uint8_t send_buffer[8*5];
//uint8_t stable_data[8];
uint8_t connected;
uint8_t connected_previous;

uint8_t measurement_data[8][5];
uint8_t stable_data[8][5];

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


int16_t analyze(uint8_t * buf){
	uint16_t result;
	uint8_t err = 0;
	uint8_t resth = ((buf[2]<<5) | (buf[1]>>3));
	uint8_t restl = ((buf[1]<<7) | (buf[0]>>1));
	if (!check_parity(restl, buf[0] & 0x1) ){
		err = 1;
		//printf("PARITY ERROR low\n\r");
	}
	if (!check_parity(resth, (buf[1]>>2) & 0x1 ) ){
		err = 1;
		//printf("PARITY ERROR high\n\r");
	}
	if (resth & ~(0x7)){
		err = 1;
		//printf("FORMAT ERROR\n\r");
	}
  //result = (((buf[2]<<5) | (buf[1]>>3)) <<8) | ((buf[1]<<7) | (buf[0]>>1));
  result =( resth <<8)|restl;
  //uint32_t result32 = (uint32_t)result;


  int32_t result32 = (int32_t)(result);
  //for (i =0;i<25;i++){
  //  result32 += result32;
  //}
  //result32 = result32 << 2;
  //result32 = result32 * 100UL * 70UL;
  //printf("3ff = %lu\n\r", result32/2047UL - 1000UL);
  //int32_t c100 = 100;
  //printf("3ff = %d\n\r", result32*c100);

  int32_t cels = result32*100L*70L/2047L - 1000L;

  //uint16_t cels = ((result * 25)>>8)*35-1000;

	if(err){
		return 0;
	}else{
		return (int16_t)cels;
	}

}

uint16_t analyze_hum_temp(uint8_t * buf){
	uint8_t tempH = buf[2];
	//uint8_t tempL = buf[1];
	return (tempH >> 1) + (tempH >> 3) + (tempH >> 6) - 40;
}

uint16_t analyze_hum_hum(uint8_t * buf){
	uint8_t capH = buf[4] & ~((1<<7)|(1<<6));
	//uint8_t capL = buf[3];
	return ((capH*3) >> 1) + (capH >>4);
}

void twi_init(){
  // set slave address
  // do NOT listen to general call
  TWAR = (SLA << 1);
  TWCR = (1<<TWEA) | (1<<TWEN);
}

void handle_communications(){
	uint8_t s, i;
  if(TWCR & (1<<TWINT)){
    //TWI interrupt
		//printf("TWSR = %x\n\r", TWSR);
    switch (TWSR){
      case 0xa8:
        // own address received, ack has been returned:
				// prepare data to send:
				
				// TODO do this with memcpy
				// TODO also send information about connected state
				for(s = 0; s<8; s++){
					for(i=0;i<5;i++){
						send_buffer[s*5 + i] = measurement_data[s][i];
					}
				}
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

void printarray(uint8_t * arr, uint8_t len){
  uint8_t i;
  for (i=0;i<len;i++){
    printf(" %x ", arr[len-1-i]);
  }
  printf("\n\r");
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
  //printf("\t\t\t!!!Result is %x\n\r", result);
  return result == 0;
}

void interpret(uint8_t * data){
	if (!(data[4] & (1<<7))){
		//printf("received: \n\r");
		//printarray(data, 5);
		//printf("\n\r");
		// this is a humidity sensor
    // check crc checksum:
		//printf("verify crc...\n\r");
    if (!verifyCRC(data, 5)){
      printf("CRC error\n\r");
    }
		//printf("done\n\r");
		uint16_t cels = analyze_hum_temp(data);
		uint16_t hum = analyze_hum_hum(data);
		printf(" T = %u, H = %u", cels, hum);

	}else{
		// this is a temperature sensor
		int16_t cels = analyze(data);
		printf("T = %d", cels);

	}

}

void do_meassurement(){
	uint8_t s;
	uint8_t i;
	connected = 0;
	for(s = 0; s<4; s++){
		// FIXME give the measurement routine a direct pointer
		// to the data. this will save ram and one copy operation
		sensor_pin_mask1 = (1<< (PC0+s));
		nsensor_pin_mask1 = ~(1<< (PC0+s));
		sensor_pin_mask2 = (1<< (PD2+s));
		nsensor_pin_mask2 = ~(1<< (PD2+s));
		meassure_start_bank1();
		meassure_start_bank2();
		for(i=0;i<60;i++){
			handle_communications();
			_delay_ms(2);
		}
		if(meassure_stop_bank1()){
			connected |= (1<<s);
		}
		//printf("icount = %u\n\r", icount);
		if(meassure_stop_bank2()){
			connected |= (1<<(4+s));
		}
		for(i=0;i<5;i++){
			measurement_data[s][i] = bytearr_bank1[i];
			measurement_data[4+s][i] = bytearr_bank2[i];
		}
	}
	if(connected_previous != connected){
		LED4_PORT &= ~(1<<LED4);
	}else{
		LED4_PORT |= (1<<LED4);
	}

	connected_previous = connected;

}

void print_interpreted_data(uint8_t ** data){
	uint8_t s;
	for(s = 0; s<8; s++){
		printf("P%u: ", s+1);
		if(connected & (1<<s) ){
			interpret(data[s]);
		}else{
			printf("X = xxxx");
		}
		printf(" | ");
	}
  printf("\n\r");

}

void loop(){
	handle_communications();
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
