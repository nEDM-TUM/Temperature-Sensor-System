#include "configuration.h"

//#include <avr/io.h>
#include <string.h>
#include "usart.h"
#include <Ethernet.h>
#include "w5100.h"
#include "socket.h"

// set debug mode
// #define DEBUG
//
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B};
uint8_t ip[] = {10,0,1, 100};
uint8_t gateway[] = {10,0,1, 1 };
uint8_t subnet[] = {255, 255, 0, 0};
uint16_t port = 8888;
uint8_t clientSock = MAX_SERVER_SOCK_NUM;
uint8_t serverSock[MAX_SERVER_SOCK_NUM];
static const char* cmd[]={"ip\n", "help\n"};
uint8_t cmdLen[]={3, 5};
static const uint8_t cmdCount = 2;
char cmdBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
uint8_t listeningSock = MAX_SERVER_SOCK_NUM;
uint8_t closedSock = MAX_SERVER_SOCK_NUM;

void execCMD(char * buff, int8_t len){
  uint8_t i = 0;
  uint8_t i2 = 0;
  uint8_t same=0;
  printf("Compare  %s\n", buff);
  // TODO lengh include \0 ???
  if(strncmp(buff, "ip", 2) == 1){
    send(sock, (uint8_t *)"IP is \n", 7);
    return;
  }
  if(strncmp(buff, "gateway", 3) == 1){
    send(sock, (uint8_t *)"GATEWAY is \n", 12);
    return;
  }
  if(strncmp(buff, "mac", 3) == 1){
    send(sock, (uint8_t *)"MAC is \n", 8);
    return;
  }
  
  // Default, usage:
  send(sock, (uint8_t *)"I want to help you\n", 19);

 }

void handleCMD(uint8_t sock){
  uint8_t buff[MAX_CMD_LEN+1];
  // TODO flush rest of buff
  int16_t ret = recv(sock, buff, MAX_CMD_LEN);
#ifdef DEBUG
  printf("Receive lenk: %d\n\r", ret);
#endif
  // TODO if a cmd in more packet???
  if(ret>0){
    buff[ret] = '\0';
    execCMD((char *)buff, (int8_t)ret);  
  }
}

void serve(){
  uint8_t i;
  for(i=0; i<MAX_SERVER_SOCK_NUM; i++){
    serverSock[i] = W5100.readSnSR(i);
#ifdef DEBUG
    printf("%u. Status: %x\n\r",i, serverSock[i]);
#endif
    switch (serverSock[i]){
      case SnSR::CLOSED:
        closedSock = i;
        break;
      case SnSR::INIT:
        printf("Init Sock: %u\n\r", i);
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
      case SnSR::IPRAW:
        printf("IP RAW Sock: %u\n\r", i);
        break;
      default:
        printf("Sock %u Status: %x\n\r", i, serverSock[i]);
    }
  }
  // TODO open more sockets
}

void setupServer() {
  
  printf("Set up server\n\r");
  //Init and config ethernet device w5100
  W5100.init();
  W5100.setMACAddress(mac);
  W5100.setIPAddress(ip);
  W5100.setGatewayIp(gateway);
  W5100.setSubnetMask(subnet);
  // Create the first server socket
  socket(0, SnMR::TCP, port, 0);
  serverSock[0] = W5100.readSnSR(0);
  if(listen(0)){
    	printf("Listened\n\r");
  } else {
    printf("not Linstend \n\r");
  }
  printf("0. Status: %x\n\r", serverSock[0]);
  while(1){
    serve();
  }
}


