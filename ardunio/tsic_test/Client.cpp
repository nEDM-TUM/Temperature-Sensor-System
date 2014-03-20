/*
  Client for transfering sensor data 
*/

#include "SPI.h"
#include "Ethernet.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
// The defined MAC address and IP address for client
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10,0,0,77};
// The server ip and port
byte server[] = {10,0,0,7};
int port = 12345;

// The client object
EthernetClient client;

// The JSON template
char json[128] = "{\"ID\":\"%s\", \"data\":\"%s\", \"acc\":\"%s\", \"time\":\"%s\"}";
/* 3 times blink as error signal (3 s)*/
// FIXME inline

inline void connectToServer(){
  if (client.connect (server, port)){
    client.println("Hello, I'm the client :)");
  }
}


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

uint16_t result;

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
	resa = bytea;
	resb = byteb;
	resc = bytec;
	PORTB = PORTB ^ (1<<PB1);
}

ISR(PCINT1_vect){
	uint8_t tval = TCNT2;
	TCNT2 = 0;
	if(PINC & (1<< PC0)){
		//rising edge
		lowtime = tval;
		bytec = (bytec << 1);
		asm("rol %0" : "=r" (byteb) : "0" (byteb));
		asm("rol %0" : "=r" (bytea) : "0" (bytea));
		bytec = bytec ^ (tval < tcrit);
	}else{
		//falling edge
		TCCR2B = (1<<CS22); // timer 2: enable with prescaler 64
		if((lowtime < (tval + 3)) && (lowtime > (tval - 3))){
			//this was start bit :)
			tcrit = tval;
			bitcount = 0;
		}
	}
}

void interrupt_init(){
	PCICR = (1<< PCIE1); //enable interrupt for PCINT[23...16]
	PCMSK1 = (1<< PCINT8);
	TIMSK2 = (1<<TOIE2); // enable overflow interript for timer0
}

inline void trySendData() {
  if (client.connected()){
		result = (((resa<<5) | (resb>>3)) <<8) | ((resb<<7) | (resc>>1));
		char buf[128];
		sprintf(buf, "resa: %x, resb: %x resc: %x", resa, resb, resc);
    client.println(buf);  
		sprintf(buf, "result: %x", result);
    client.println(buf);  
		uint16_t cels = ((result * 25)>>8)*35-1000;
		sprintf(buf, "cels: %u", cels);
    client.println(buf);  
    // Do success blink after send Data
    // The blink costs 1 second
    //sucBlink();
    // FIXME When server write sth. to the client,
    // that means, sth. may be wrong,
    // do error blink, which takes 3 seconds!!!
    if(client.available()){
      client.flush();
    }
		_delay_ms(1000);
  }else{
    // If connect failed, do error blink every 5 seconds 
    //errorBlink();
	  _delay_ms(4000);
    connectToServer();
  }
}

int main() {
  init();
	DDRB = (1<<PB1);
	TCCR2A = 0;
	TCCR2B = 0;
	interrupt_init();


  // initialize the ethernet device
  Ethernet.begin(mac, ip);

  connectToServer();

  while (1){
    trySendData();
  }
}

