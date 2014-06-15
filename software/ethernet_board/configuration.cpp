#include "configuration.h"

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usart.h"
#include "socket.h"
#include "sock_stream.h"
#include <Ethernet.h>
#include "w5100.h"
// set debug mode
// #define DEBUG


struct config cfg = {
  /*.mac = */{0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B},
  /*.ip = */{10,0,1, 100},
  /*.subnet = */16,
  /*.gw = */{10, 0, 1, 1},
  /*.port = */8888,
  /*.ip_db = */{10, 0, 1, 99},
  /*.port_db = */8888
};
uint8_t clientSock = MAX_SERVER_SOCK_NUM;
uint8_t serverSock[MAX_SERVER_SOCK_NUM];

uint8_t listeningSock = MAX_SERVER_SOCK_NUM;
uint8_t closedSock = MAX_SERVER_SOCK_NUM;

uint8_t data_request[MAX_SERVER_SOCK_NUM] = {0};
uint32_t measure_interval = 1000;

char receiveBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
uint8_t receiveBuffPointer[MAX_SERVER_SOCK_NUM] = {0}; // Point to a byte, which will be written

void send_result(struct dummy_packet * packets);
void (*twi_access_fun)();

uint8_t ui_state = UI_READY;


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


void beginService() {
#ifdef EEPROM
  // TODO read eeprom 
#endif
  uint8_t sn[4];
  //Init and config ethernet device w5100
  W5100.init();
  W5100.setMACAddress(cfg.mac);
  W5100.setIPAddress(cfg.ip);
  W5100.setGatewayIp(cfg.gw);
  toSubnetMask(cfg.subnet, sn);
  W5100.setSubnetMask(sn);
  // TODO reset client to db
  // Create the first server socket
  socket(0, SnMR::TCP, cfg.port, 0);
  serverSock[0] = W5100.readSnSR(0);
  while(!listen(0)){
    // wait a second and try again
    _delay_ms(1000);
  }
}



void dataAvailable(struct dummy_packet * received, uint8_t src_addr){
	uint8_t i;
	for(i=0; i< MAX_SERVER_SOCK_NUM; i++){
		// TODO: check if socket is still connected.
		if (data_request[i]){
			stream_set_sock(i);
			fprintf(&sock_stream, "%u :: ", src_addr);
			send_result(received);
		}
	}
}

void serve(){
  uint8_t i;
  closedSock = MAX_SERVER_SOCK_NUM;
  listeningSock = MAX_SERVER_SOCK_NUM;
  for(i=0; i<MAX_SERVER_SOCK_NUM; i++){
    serverSock[i] = W5100.readSnSR(i);
    //printf("%u. Status: %x\n\r",i, serverSock[i]);
#ifdef DEBUG
    printf("%u. Status: %x\n\r",i, serverSock[i]);
#endif
    switch (serverSock[i]){
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
        handleCMD(i);
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
#ifdef DEBUG
        printf("Sock %u Status: %x\n\r", i, serverSock[i]);
#endif
    }
  }
#ifdef DEBUG
  printf("Listening socket %d, closed socket %d\n\r",listeningSock, closedSock);
#endif
  if(listeningSock == MAX_SERVER_SOCK_NUM && closedSock < MAX_SERVER_SOCK_NUM){
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
				// we again accept commands
				ui_state = UI_READY;
				twi_free_bus();
			}
			break;
	}
}

void setupServer() {
  sock_stream_init();
  printf("Set up server\n\r");
  beginService();
}


