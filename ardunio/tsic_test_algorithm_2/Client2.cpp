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


uint8_t tl = 255;
uint8_t tsd = 255;
uint8_t bindex= 1;
uint8_t bits[] = {0, 0};
uint8_t parity[] = {0, 0};
uint8_t resparity;

uint8_t deb1 = 255;
uint8_t deb2 = 255;

uint16_t result;
#define error 3
#define error2 6
inline void storeBit(uint8_t bindex, uint8_t bit){
  bits[bindex] << 1;
  bits[bindex] |= parity[bindex];
  parity[bindex]=bit;
}

ISR(TIMER2_OVF_vect){
	TCCR2B = 0; //disable timer
  bindex=1;
	result = (bits[bindex] <<8)|bits[bindex-1];
	resparity = (parity[bindex] << 1) | parity[bindex];
}
// Interrupt handler of PCIE1 (PCINIT[14...8]!!!)
ISR(PCINT1_vect){
  // Read timer 2
	uint8_t tc = TCNT2;
  // Reset timer 2
	TCNT2 = 0;
  
	if(PINC & (1<< PC0)){
		// PC0 is 1 -> rising edge
    // Save the timer counter plus error
    tl = tc + error;
	}else{
		// PC0 is 0 -> falling edge
		if(tl<tc){
      // It is one
      if(tl>tsd){ 
        // It is actually 0 STOP
        storeBit(bindex, 0);
      } else{
        storeBit(bindex, 1);
      }
    }else{
      if(tl-tc > error2){
        // It is zero
        storeBit(bindex, 0);
  // TODO  debug, LED blink
	PORTB = PORTB ^ (1<<PB1);
      }else{
			  //this was start bit :)
        bindex^=bindex;
        tsd = tc;
        if(bindex){
          parity[bindex] = parity[bindex-1];
          bits[bindex] = parity[bindex-1];
        }
      }
    }
	TCCR2B = (1<<CS22); // start timer 2: enable with prescaler 64
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
  // Reset timer 2
	TCNT2 = 0;
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

