/*
 Chat  Server
 
 A simple server that distributes any incoming messages to all
 connected clients.  To use telnet to  your device's IP address and type.
 You can see the client's input in the serial monitor as well.
 Using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */

#include "SPI.h"
#include "Ethernet.h"
#include <avr/io.h>
#include <util/delay.h>
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10,0,0,77);
IPAddress gateway(10,0,0,1);
IPAddress subnet(255, 255,0,0);


// telnet defaults to port 23
EthernetServer server(23);
boolean alreadyConnected = false; // whether or not the client was connected previously

void test(){
	_delay_ms(1000);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(200);

}

int main() {
	DDRB = (1<<PB1);


	test();
  // initialize the ethernet device
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  //Ethernet.begin(mac);
  // start listening for clients
	test();
  server.begin();
 // Open serial communications and wait for port to open:
	test();

  while (1){
    loop();
  }
}

void loop() {
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(100);
  // wait for a new client:
  EthernetClient client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {
    if (!alreadyConnected) {
      // clead out the input buffer:
      client.flush();    
      client.println("Hello, client!"); 
      alreadyConnected = true;
    } 

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      // echo the bytes back to the client:
      server.write(thisChar);
      // echo the bytes to the server as well:
    }
  }
}



