#include "networking.h"

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usart.h"
#include "config.h"
#include "socket.h"
#include "sock_stream.h"
#include <Ethernet.h>
#include "w5100.h"
// set debug mode
// #define DEBUG

uint8_t listeningSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;
uint8_t closedSock = MAX_SERVER_SOCK_NUM + FIRST_SERVER_SOCK;

uint8_t data_request[MAX_SERVER_SOCK_NUM] = {0};
uint32_t measure_interval = 1000;

char receiveBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
uint8_t receiveBuffPointer[MAX_SERVER_SOCK_NUM] = {0}; // Point to a byte, which will be written

void send_result(struct dummy_packet * packets);
void (*twi_access_fun)();

uint8_t ui_state = UI_READY;

const char UintDot_4Colon[] PROGMEM = "%u.%u.%u.%u:%u";
// TODO make it configurable and store in eeprom
const char Database[] PROGMEM = "nedm%2Ftemperature";
const char Cookie[] PROGMEM = "c29sYXI6NTIwNEU4OTA66nT2KHyr9n5RrouelBtLnBru00c"; 


int8_t toSubnetMask(uint8_t subnet, uint8_t* addr){
  int8_t indexByte = 0;
  int8_t indexBit = 0;
  while((subnet >> 3) >0 ){
  printf("Subnet: %d %d\n\r", subnet, subnet-8<subnet);
    addr[indexByte] = 255;
    indexByte++;
    subnet -= 8;
  }
  if(indexByte>=4){
    return 0;
  }
  while(subnet != 0 ){
    addr[indexByte] |= 1 << indexBit;
    indexBit ++;
    subnet--;
    if(indexBit>=8){
      return 0;
    }
  }
  return 1;
}

void sendResultToDB(struct dummy_packet * packets){
  uint8_t index;
  stream_set_sock(DB_CLIENT_SOCK);
  fputs_P(PSTR("POST http://"), &sock_stream);
  fprintf_P(&sock_stream, UintDot_4Colon, cfg.ip_db[0], cfg.ip_db[1], cfg.ip_db[2], cfg.ip_db[3], cfg.port_db);
  fputs_P(Database, &sock_stream);
  // TODO to compute content length??? fix??
  fputs_P(PSTR("/_design/nedm_default/_update/insert_with_timestamp HTTP/1.0\nContent-Length: ###\nCookie: AuthSession="), &sock_stream);
  fputs_P(Cookie, &sock_stream);
  fputs_P(PSTR("\nX-CouchDB-WWW-Authenticate: Cookie\nContent-Type: application/json\n\n"), &sock_stream);
  // TODO convert packet data to json format
  //fprintf_P(&sock_stream, PSTR("{\"%s\"["), packet_name_or_addr);
  //for(index = 0; index < 8; index++){
  //  if(datavalid??){
  //    if(!first??){
  //      fputs_P(PSTR(","), &sock_stream);

  //    } 
  //    fprintf_P(&sock_stream, PSTR("{\"%s\":%u.%u,\"%s\":\"%s\"i}"), valueAttribute, value, otherAttribute, otherValue);
  //  }
  //}
  //fputs_P(PSTR("]}"), &sock_stream);
  // TODO if ok to send small packet for http
  sock_stream_flush();
  
}

void beginService() {
  config_read(&cfg);
  uint8_t sn[4];
  //Init and config ethernet device w5100
	printf("ip: %u.%u.%u.%u\n\r", cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3]);
  W5100.init();
  W5100.setMACAddress(cfg.mac);
  W5100.setIPAddress(cfg.ip);
  W5100.setGatewayIp(cfg.gw);
  toSubnetMask(cfg.subnet, sn);
  W5100.setSubnetMask(sn);
  // Create client to db
  // Port of the client can be arbitary, but it should be different then the port of ui server
  if(!socket(DB_CLIENT_SOCK, SnMR::TCP, cfg.port+1, 0)){
    socket(DB_CLIENT_SOCK, SnMR::TCP, cfg.port-1, 0);
  }
  connect(DB_CLIENT_SOCK, cfg.ip_db, cfg.port_db);
  // Create the first server socket
  socket(FIRST_SERVER_SOCK, SnMR::TCP, cfg.port, 0);
  while(!listen(FIRST_SERVER_SOCK)){
    // wait a second and try again
    _delay_ms(1000);
  }
}



void dataAvailable(struct dummy_packet * received, uint8_t src_addr){
	uint8_t i;
	for(i=FIRST_SERVER_SOCK; i< MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; i++){
		// TODO: check if socket is still connected.
		if (data_request[i-FIRST_SERVER_SOCK]){
			stream_set_sock(i);
			fprintf(&sock_stream, "%u :: ", src_addr);
			send_result(received);
		}
	}
	sock_stream_flush();
}

void serve(){
  uint8_t i;
  uint8_t snSR;
  uint8_t cmd_state;
  closedSock = MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK;
  listeningSock = MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK;
  for(i=FIRST_SERVER_SOCK; i<MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK; i++){
    snSR = W5100.readSnSR(i);
#ifdef DEBUG
    printf("%u. Status: %x\n\r",i, snSR);
#endif
    switch (snSR){
      case SnSR::CLOSED:
        closedSock = i;
        break;
      case SnSR::INIT:
#ifdef DEBUG
        printf("Init Sock: %u\n\r", i);
#endif
        break;
      case SnSR::LISTEN:
        listeningSock = i;
        break;
      case SnSR::ESTABLISHED:
        cmd_state = handleCMD(i);
				if(cmd_state == SUSPEND){
					// handleCMD requested, to not accept new commands,
					// so we return here:
					return;
				}
        break;
      case SnSR::CLOSING:
        // TODO if readCMD??
        printf("Closing Sock: %u\n\r", i);
        break;
      case SnSR::TIME_WAIT:
        printf("Time wait Sock: %u\n\r", i);
        break;
      case SnSR::CLOSE_WAIT:
        // TODO implement see Arduino Ethernet library
        printf("Close wait Sock: %u\n\r", i);
        break;
      default:
        break;
    }
  }
#ifdef DEBUG
  printf("Listening socket %d, closed socket %d\n\r",listeningSock, closedSock);
#endif
  if(listeningSock == MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK && closedSock < MAX_SERVER_SOCK_NUM+FIRST_SERVER_SOCK){
    socket(closedSock, SnMR::TCP, cfg.port, 0);    
    listen(closedSock);
  }
}

void ui_loop(){
	switch(ui_state){
		case UI_READY:
			serve();
			break;
		case UI_TWILOCK:
			// we do not process new socket inputs, as we have to wait for the twi bus to become ready:
			if(twi_try_lock_bus()){
				// we aquired the bus
				// now we have to do our access as fast as possible, as others might wait for bus access:
				// call command handler, who waits for access to the bus:
				twi_access_fun();
				sock_stream_flush();
				// we again accept commands
				ui_state = UI_READY;
				twi_free_bus();

				// handle other already existing commands in the buffer
				handleCMD(stream_get_sock());
			}
			break;
	}
}

void setupServer() {
  sock_stream_init();
#ifdef DEBUG
  printf("Set up server\n\r");
#endif
  beginService();
}


