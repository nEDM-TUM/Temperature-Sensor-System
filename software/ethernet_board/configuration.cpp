#include "configuration.h"

//#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usart.h"
#include <Ethernet.h>
#include "w5100.h"
#include "socket.h"
#include "collector_twi.h"
#include "packet.h"
// set debug mode
// #define DEBUG
//
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
uint8_t mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0xE3, 0x5B};
uint8_t ip[] = {10,0,1, 100};
uint8_t gw[] = {10,0,1, 1 };
uint8_t subnet = 16;
uint8_t subnetM[] = {255, 255, 0, 0};
uint16_t port = 8888;
uint8_t ip_db[] = {10,0,1, 100};
uint16_t port_db = 8888;
uint8_t clientSock = MAX_SERVER_SOCK_NUM;
uint8_t serverSock[MAX_SERVER_SOCK_NUM];

uint8_t listeningSock = MAX_SERVER_SOCK_NUM;
uint8_t closedSock = MAX_SERVER_SOCK_NUM;

char receiveBuff[MAX_SERVER_SOCK_NUM][MAX_CMD_LEN];
uint8_t writeBuffPointer[MAX_SERVER_SOCK_NUM] = {0}; // Point to a byte, which will be written

struct cmd{
  const char * name;
  void (*handle)(uint8_t, char*);
};

void handleIp(uint8_t sock, char* paramsStr);
void handlePort(uint8_t sock, char* paramsStr);
void handleMac(uint8_t sock, char* paramsStr);
void handleGw(uint8_t sock, char* paramsStr);
void handleReset(uint8_t sock, char* paramsStr);
void handleIpDB(uint8_t sock, char* paramsStr);
void handlePortDB(uint8_t sock, char* paramsStr);
void handleTwiaddr(uint8_t sock, char* paramsStr);
void handleDoMeasurement(uint8_t sock, char* paramsStr);

//Response buff block
FILE  res_stream;
char resBuff[MAX_RESPONSE_LEN];
uint8_t resBuffPointer = 0;
uint8_t currResSock = MAX_SERVER_SOCK_NUM;

const uint8_t cmdLen = 9;
struct cmd cmds[]={
  {"ip", handleIp},
  {"port", handlePort},
  {"mac", handleMac},
  {"gw", handleGw},
  {"reset", handleReset},
  {"ip-db", handleIpDB},
  {"port-db", handlePortDB},
  {"twiaddr", handleTwiaddr},
  {"m", handleDoMeasurement}
};

const char WillSet[] PROGMEM = " will be set to ";
const char IfUpdate[] PROGMEM = ", if you input 'reset'\n";
const char UpdateOption[] PROGMEM = ", update option: ";
const char WillReset[] PROGMEM = "The ethernet service will be reset, the future login is ";
const char Addr[] PROGMEM = " <addr>\n";

int res_flush(){
  // TODO send

  resBuffPointer =0;
}
int res_putchar(char c, FILE *stream){
  if(resBuffPointer == MAX_RESPONSE_LEN){
    res_flush();
  }
  resBuff[resBuffPointer++] = c;
}

void res_set_sock(uint8_t sock){
  res_flush();
  currResSock = sock;
}


void res_init(){
  fdev_setup_stream(&res_stream, res_putchar, NULL, _FDEV_SETUP_WRITE);
  //fdev_set_udata(res_stream, u);
}

int8_t toSubnetMask(uint8_t subnet, uint8_t* addr){
  int8_t indexByte = 0;
  int8_t indexBit = 0;
  printf("Subnet Mask: %d.%d.%d.%d\n\r", subnetM[0], subnetM[1], subnetM[2], subnetM[3]);
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
  printf("Subnet Mask: %d.%d.%d.%d\n\r", subnetM[0], subnetM[1], subnetM[2], subnetM[3]);
  return 1;
}

int8_t convertParamToBytes(char * buff, int8_t len, uint8_t * params){
  int8_t index;
  int8_t paramIndex =0;
  int8_t paramCounter =0;
  char paramBuff[MAX_PARAM_LEN];
  for(index=0; index<len; index++){
    if(paramIndex<MAX_PARAM_LEN && buff[index]>=48 && buff[index]<=57){
      paramBuff[paramIndex] = buff[index];
      paramIndex++;
    }else if(buff[index] == '\0'){
      break;
    }else{
      if(paramIndex>0){
        paramBuff[paramIndex] = '\0';
        // FIXME for mac hex 
        params[paramCounter] = atoi(paramBuff);
        paramCounter++;
        paramIndex = 0;
      }
    }
  }
  if(paramIndex>0){
    paramBuff[paramIndex] = '\0';
    params[paramCounter] = atoi(paramBuff);
    paramCounter++;
  }
  return paramCounter;
}

void beginService() {
  //Init and config ethernet device w5100
  toSubnetMask(subnet, subnetM);
  W5100.init();
  W5100.setMACAddress(mac);
  W5100.setIPAddress(ip);
  W5100.setGatewayIp(gw);
  W5100.setSubnetMask(subnetM);
  // TODO reset client to db
  // Create the first server socket
  socket(0, SnMR::TCP, port, 0);
  serverSock[0] = W5100.readSnSR(0);
  while(!listen(0)){
    // wait a second and try again
    _delay_ms(1000);
  }
}


char* cmpCMD(char* cmdstr, const char * cmd){
  while(*cmd!='\0' && cmdstr!='\0' && *cmd == *cmdstr){
    cmd++;
    cmdstr++;
  }
  if(*cmd=='\0'){
    if(*cmdstr=='\0'){
      return cmdstr; // equal, no param
    }else if(*cmdstr == ' '){
      cmdstr ++;
      return cmdstr; // equal, contains param
    }
  }
  return NULL; // not equal
}

void execCMD(uint8_t sock, char * buff, int8_t len){
  int16_t resLen=0;
  uint8_t params[MAX_PARAM_COUNT];
  int8_t paramCounter =0;
  int8_t index;
  char * cmpedBuff;
  struct cmd cmd;
  printf("Compare  %s\n\r", buff);
  for(index= 0; index<cmdLen; index++){
    cmd = cmds[index];
    cmpedBuff = cmpCMD(buff, cmd.name);
    if(cmpedBuff!=NULL){
      cmd.handle(sock, cmpedBuff);
      return;
    }
  }
  resLen = sprintf(resBuff, "Usage: TODO\n");
  send(sock, (uint8_t *)resBuff, resLen);
  return;
}

int8_t sendBuff(uint8_t sock, char c){

  
}

void handleIp(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t resLen=0;
  uint8_t ipTMP[4];
  uint8_t subnetTMP; 
  if(*paramsStr == '\0'){
    resLen = sprintf(resBuff, "ip: %d.%d.%d.%d/%d\n", ip[0], ip[1], ip[2], ip[3], subnet);
  }else{
    if(sscanf(paramsStr, "%u.%u.%u.%u/%u", ipTMP[0], ipTMP[1], ipTMP[2], ipTMP[3], subnet)==5){
      for(index=0; index<4; index++){
        ip[index] = ipTMP[index];
      }
      resLen = sprintf(resBuff, "ip%S%d.%d.%d.%d/%d%S", WillSet, ip[0], ip[1], ip[2], ip[3], subnet, IfUpdate);
    } else {
      resLen = sprintf(resBuff, "ip: %d.%d.%d.%d/%d%Sip addr/subnet\n", ip[0], ip[1], ip[2], ip[3], subnet, UpdateOption);
    }
  }
  send(sock, (uint8_t *)resBuff, resLen);
}

uint8_t print4dotarr(char * buf, uint8_t * arr){
  return sprintf(buf, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
}

void handleGw(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t resLen=0;
  uint8_t gwTMP[4];
  if(*paramsStr == '\0'){
    resLen = sprintf(resBuff, "gw: ");
    resLen += print4dotarr(resBuff + resLen, gw);
    resLen += sprintf(resBuff, "\n");
  }else{
    if(sscanf(paramsStr, "%u.%u.%u.%u", gwTMP[0], gwTMP[1], gwTMP[2], gwTMP[3])==4){
      for(index=0; index<4; index++){
        gw[index] = gwTMP[index];
      }
      resLen = sprintf(resBuff, "gw%S%d.%d.%d.%d%S", WillSet, gw[0], gw[1], gw[2], gw[3], IfUpdate);
    }else{
        resLen = sprintf(resBuff, "gw: %d.%d.%d.%d%Sgw%S", gw[0], gw[1], gw[2], gw[3], UpdateOption, Addr);
    }
  }
  send(sock, (uint8_t *)resBuff, resLen);
}

void handleMac(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t resLen=0;
  uint8_t macTMP[6];
  if(*paramsStr == '\0'){
      resLen = sprintf(resBuff, "mac: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }else{
    if(sscanf(paramsStr, "%x:%x:%x:%x:%x:%x", macTMP[0], macTMP[1], macTMP[2], macTMP[3], macTMP[4], macTMP[5])==6){
      for(index=0; index<4; index++){
        mac[index] = macTMP[index];
      }
      resLen = sprintf(resBuff, "mac%S%x:%x:%x:%x:%x:%x%S", WillSet, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], IfUpdate);
    }else{
      resLen = sprintf(resBuff, "mac: %x:%x:%x:%x:%x:%x%Smac%S\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], UpdateOption, Addr);
    }
  }
  send(sock, (uint8_t *)resBuff, resLen);
}

void handlePort(uint8_t sock, char* paramsStr){
  int16_t resLen=0;
  if(*paramsStr == '\0'){
    resLen = sprintf(resBuff, "port: %d\n", port);
  }else{
    if(sscanf(paramsStr, "%u", port)==1){
      resLen = sprintf(resBuff, "port%S%d%S", WillSet, port, IfUpdate);
    }else{
      resLen = sprintf(resBuff, "port: %d%Sport number\n", port, UpdateOption);
    }
  }
  send(sock, (uint8_t *)resBuff, resLen);
}

void handleReset(uint8_t sock, char* paramsStr){
  int16_t resLen=0;
  uint8_t index;
  resLen = sprintf(resBuff, "%S%d.%d.%d.%d:%d\n",WillReset, ip[0], ip[1], ip[2], ip[3], port);
  for(index= 0; index<MAX_SERVER_SOCK_NUM; index++){
    if(W5100.readSnSR(index) == SnSR::ESTABLISHED){
      send(index, (uint8_t *)resBuff, resLen);
    }
    disconnect(index);
  }
  // Wait a second to close all sockets
  _delay_ms(1000);
  // TODO LEDs action
  for(index= 0; index<MAX_SERVER_SOCK_NUM; index++){
    if(W5100.readSnSR(index) != SnSR::CLOSED){
      // If a socket is still not closed, force it
      close(index);
    }
  }
  beginService();
}

void handleIpDB(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t resLen=0;
  uint8_t ipTMP[4];
  uint8_t subnetTMP; 
  if(*paramsStr == '\0'){
    resLen = sprintf(resBuff, "db ip: %d.%d.%d.%d/%d\n", ip_db[0], ip_db[1], ip_db[2], ip_db[3], subnet);
  } else {
    if(sscanf(paramsStr, "%u.%u.%u.%u/%u", ipTMP[0], ipTMP[1], ipTMP[2], ipTMP[3], subnet)==5){
      for(index=0; index<4; index++){
        ip_db[index] = ipTMP[index];
      }
      resLen = sprintf(resBuff, "db ip %S%d.%d.%d.%d/%d%S", WillSet, ip_db[0], ip_db[1], ip_db[2], ip_db[3], subnet, IfUpdate);
    } else {
      resLen = sprintf(resBuff, "db ip: %d.%d.%d.%d/%d%Sip-db%S", ip_db[0], ip_db[1], ip_db[2], ip_db[3], subnet, UpdateOption, Addr);
    }
  }
  send(sock, (uint8_t *)resBuff, resLen);
}

void handlePortDB(uint8_t sock, char* paramsStr){
  int8_t index;
  int16_t resLen=0;
  if(*paramsStr == '\0'){
    resLen = sprintf(resBuff, "db port: %d\n", port);
  }else{
    if(sscanf(paramsStr, "%u", port)==1){
      resLen = sprintf(resBuff, "db port%S%d%S", WillSet, port, IfUpdate);
    }else{
      resLen = sprintf(resBuff, "db port: %d%Sport-db number\n", port, UpdateOption);
    }
  } 
  send(sock, (uint8_t *)resBuff, resLen);
}

void send_result(struct dummy_packet * packets, uint8_t sock){
  char buffer[10];
	uint8_t s;
  uint8_t resLen;
	int16_t temp;
  for (s=0;s<8;s++){
    resLen = sprintf(buffer, " | P%u: ", s+1);
    send(sock, (uint8_t *)buffer, resLen);
    if(packets[s].header.error && packets[s].header.connected){
      resLen = sprintf(buffer, " ERROR ");
      send(sock, (uint8_t *)buffer, resLen);
    }
    if(packets[s].header.connected){
      switch(packets[s].header.type){
        case PACKET_TYPE_TSIC:
          temp =  ((struct tsic_packet *)(packets) )[s].temperature;
          resLen = sprintf(buffer, "T = %d.%02d", temp/100, temp%100);
          send(sock, (uint8_t *)buffer, resLen);
          //printf(" T = %d", ( (struct tsic_packet *)(packets) )[s].temperature);
          break;
        case PACKET_TYPE_HYT:
          resLen = sprintf(buffer, "T = %d", ( (struct hyt_packet *)(packets) )[s].temperature);
          send(sock, (uint8_t *)buffer, resLen);
          resLen = sprintf(buffer, " H = %d", ( (struct hyt_packet *)(packets) )[s].humidity);
          send(sock, (uint8_t *)buffer, resLen);
          break;
        default:
          resLen = sprintf(buffer, "---?---");
          send(sock, (uint8_t *)buffer, resLen);
          break;
      }
    }else{
      resLen = sprintf(buffer, "---nc---");
      send(sock, (uint8_t *)buffer, resLen);
    }
  }
}

void handleDoMeasurement(uint8_t sock, char* paramsStr){
	printf("handler do measure\n\r");
  twi_start_measurement(0);
	uint8_t iaddr;
	uint8_t addr;
  uint8_t state;
  struct dummy_packet received[8];
	for (iaddr=0;iaddr<num_boards;iaddr++){
		addr = scanresults[iaddr];
		printf("# %u # ", addr);
		state = twi_receive_data(addr, ((uint8_t*)received),8*sizeof(struct dummy_packet));
    if (state){
      send_result(received, sock);
    }
		send(sock, (uint8_t *)"\n", 1);
  }


}
int8_t handleScan(uint8_t sock, char * paramsStr){

	uint8_t i;
	char buf[10];
	num_boards = twi_scan(scanresults, 20);
	send(sock, (uint8_t *)"found boards\n", 13);
	printf("found boards: ");
	for (i=0;i<num_boards;i++){
		printf("%u", scanresults[i]);
		uint8_t t;
		t = sprintf(buf, "%u", scanresults[i]);
		send(sock, (uint8_t *)buf, t);

	}
	printf("\n\r");
	send(sock, (uint8_t *)"\n", 1);
}

void handleTwiaddr(uint8_t sock, char * paramsStr){
  uint8_t resLen;
  uint8_t synerr = 1;
  if(paramsStr[0]!='\0'){
    uint8_t old_addr, new_addr;
    if(sscanf(paramsStr, "%u %u", &old_addr, &new_addr) ==2){
			printf("par %u, %u", old_addr, new_addr);
			printf("|%s|", paramsStr);
      //if(buff[4]=='g'){
      synerr = 0;
      if(twi_set_address(old_addr, new_addr)){
        resLen = sprintf(resBuff, "success\n");
      }else{
        resLen = sprintf(resBuff, "failed\n");
      }
    }
    }
    if (synerr){
      resLen = sprintf(resBuff, "Usage: twiaddr <old>%S", Addr);
    }
    send(sock, (uint8_t *)resBuff, resLen);

}

inline void sendError(uint8_t sock){
  send(sock, (uint8_t *)"Error!\n", 7);
}

void handleCMD(uint8_t sock){
  uint8_t b;
  while(recv(sock, &b, 1) >0){
    if(b=='\n' || b==';'){
      receiveBuff[sock][writeBuffPointer[sock]] = '\0';
      execCMD(sock, receiveBuff[sock], writeBuffPointer[sock]);
      writeBuffPointer[sock] = 0;
    }else if(writeBuffPointer[sock] < MAX_CMD_LEN-1){
      // XXX The first char should be alphabet in low case 
      if(writeBuffPointer[sock] == 0 && (b<97 || b>122)){
        continue;
      }
      receiveBuff[sock][writeBuffPointer[sock]] = b;
      writeBuffPointer[sock]++;
      writeBuffPointer[sock] %= MAX_CMD_LEN;
    }else{
      sendError(sock); 
      while(W5100.getRXReceivedSize(sock) && recv(sock, &b, 1) >0){
        ;
      }
      writeBuffPointer[sock] = 0;
      break;
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
    socket(closedSock, SnMR::TCP, port, 0);    
    listen(closedSock);
  }
}

void setupServer() {
  printf("Set up server\n\r");
  beginService();
  while(1){
    serve();
  }
}


