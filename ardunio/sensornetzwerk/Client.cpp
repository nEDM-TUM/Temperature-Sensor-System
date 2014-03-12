/*
  Client for transfering sensor data 
*/

#include "SPI.h"
#include "Ethernet.h"
#include <avr/io.h>
#include <util/delay.h>
// The defined MAC address and IP address for client
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 10,0,0,77};
// The server ip and port
byte server[] = {10,0,1,1};
int port = 12345;

// The client object
EthernetClient client;

// The JSON template
{"value":{"name1": xxx, "name2": xxx ....}}
char json[128] = "{\"ID\":\"%s\", \"data\":\"%s\", \"acc\":\"%s\", \"time\":\"%s\"}";

/* flash led 3 times as error signal (3 s)*/
// FIXME inline
inline void errorFlash(){
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(150);
	PORTB = 0<<PB1;
	_delay_ms(100);
}

/* flash led one time as a success signal (1 s) */
// FIXME inline
inline void sucFlash(){
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(500);
	PORTB = PORTB ^ (1<<PB1);
	_delay_ms(500);
	PORTB = 0<<PB1;
}

inline void connectToServer(){
  if (client.connect (server, port)){
    client.println("Hello, I'm the client :)");
  }
}

// Convert data to JSON format
// TODO parameter types
inline char * formatJSON(char* id, char* data, char* acc, char* time){
  char buf[128];
  sprintf(buf, json, id, data, acc, time);
  return buf;
}

inline char * getJSONdata(){
  return formatJSON("XX", "xxxx", "0.1", "dd-MM-yyyy hh:mm:ss"); 
}

inline void trySendData() {
  if (client.connected()){
    client.println(getJSONdata());  
    // Indicate success after data has been sent
    // this blocks for 1 second
    sucFlash();
    // FIXME When server writes sth. to the client,
    // that means, sth. might be wrong,
    // indicate error, which takes 3 seconds!!!
    if(client.available()){
      client.flush();
      errorFlash();
    }
  }else{
    // If connect failed, indicate error every 5 seconds 
    errorFlash();
	  _delay_ms(4000);
    connectToServer();
  }
}

int main() {
  init();
	DDRB = (1<<PB1);

  // initialize the ethernet device
  Ethernet.begin(mac, ip);

  connectToServer();

  while (1){
    trySendData();
  }
}

