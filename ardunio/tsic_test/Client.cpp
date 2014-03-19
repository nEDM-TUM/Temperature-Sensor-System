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
uint8_t byteh = 255;
uint8_t bytel = 255;
uint8_t tcrit;
uint8_t bitcount = 255;
uint8_t parity;
uint8_t resparity;

uint16_t result;

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
	result = (byteh <<8)|bytel;
	resparity = parity;
  // debug, LED blink
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
		if(bitcount != 8){
			bytel = (bytel << 1);
			bytel = bytel ^ (tval < tcrit);
			bitcount++;
		}else{
			parity = (parity << 1);
			parity = parity ^ (tval < tcrit);
		}
	}else{
		// PC0 is 0 -> falling edge
		TCCR2B = (1<<CS22); // start timer 2: enable with prescaler 64
    // FIXME find appropriate range
		if((lowtime < (tval + 3)) && (lowtime > (tval - 3))){
			//this was start bit :)
			tcrit = tval;
			byteh = bytel;
			bitcount = 0;
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

inline void trySendData() {
  if (client.connected()){
		char buf[128];
		sprintf(buf, "deb1: %d, deb2: %d", deb1, deb2);
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
	interrupt_init();


  // initialize the ethernet device
  Ethernet.begin(mac, ip);

  connectToServer();

  while (1){
    trySendData();
  }
}

